#pragma once

#include "Hazel/Core/Core.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Hazel {

	template<typename T>
	using CommandNormalType = std::remove_cv_t<std::remove_reference_t<T>>;

	class CommandQueue {
	public:
		static constexpr size_t MAX_COMMAND_SIZE = 1024;
		static constexpr uint32_t DEFAULT_COMMAND_MEM_SIZE_KB = 64;

	private:
		struct CommandBase {
			bool sync = false;

			virtual void Execute() = 0;
			virtual void Destroy() = 0;
			virtual ~CommandBase() = default;

			explicit CommandBase(bool isSync)
				: sync(isSync) {}
		};

		template<typename T, typename M, bool NeedsSync, typename... Args>
		struct Command : CommandBase {
			T* instance = nullptr;
			M method;
			std::tuple<CommandNormalType<Args>...> args;

			template<typename... FwdArgs>
			Command(T* pInstance, M pMethod, FwdArgs&&... pArgs)
				: CommandBase(NeedsSync), instance(pInstance), method(pMethod), args(std::forward<FwdArgs>(pArgs)...) {}

			void Execute() override {
				ExecuteImpl(std::index_sequence_for<Args...>{});
			}

			void Destroy() override {
				this->~Command();
			}

		private:
			template<size_t... I>
			void ExecuteImpl(std::index_sequence<I...>) {
				(instance->*method)(std::move(std::get<I>(args))...);
			}
		};

		template<typename T, typename M, typename R, typename... Args>
		struct CommandRet : CommandBase {
			T* instance = nullptr;
			M method;
			R* ret = nullptr;
			std::tuple<CommandNormalType<Args>...> args;

			CommandRet(T* pInstance, M pMethod, R* pRet, CommandNormalType<Args>... pArgs)
				: CommandBase(true), instance(pInstance), method(pMethod), ret(pRet), args(std::move(pArgs)...) {}

			void Execute() override {
				*ret = ExecuteImpl(std::index_sequence_for<Args...>{});
			}

			void Destroy() override {
				this->~CommandRet();
			}

		private:
			template<size_t... I>
			R ExecuteImpl(std::index_sequence<I...>) {
				return (instance->*method)(std::move(std::get<I>(args))...);
			}
		};

		template<typename T, typename... Args>
		void CreateCommand(Args&&... pArgs) {
			constexpr uint64_t allocSize = AlignTo8(sizeof(T));
			static_assert(allocSize < UINT32_MAX, "Command type too large.");

			const uint64_t size = m_CommandMem.size();
			m_CommandMem.resize(size + allocSize + sizeof(uint64_t));
			std::memcpy(&m_CommandMem[size], &allocSize, sizeof(uint64_t));

			void* cmd = &m_CommandMem[size + sizeof(uint64_t)];
			new (cmd) T(std::forward<Args>(pArgs)...);

			m_Pending.store(true);
		}

		template<typename T, bool NeedsSync, typename... Args>
		void PushInternal(Args&&... pArgs) {
			std::unique_lock<std::mutex> lock(m_Mutex);
			CreateCommand<T>(std::forward<Args>(pArgs)...);
			m_PendingCondition.notify_one();

			if constexpr (NeedsSync) {
				m_SyncTail++;
				WaitForSync(lock);
			}
		}

		void PreventSyncWraparound() {
			const bool safeToReset = (m_SyncAwaiters == 0);
			const bool alreadySynced = (m_SyncHead == m_SyncTail);
			if (safeToReset && alreadySynced) {
				m_SyncHead = 0;
				m_SyncTail = 0;
			}
		}

		void WaitForSync(std::unique_lock<std::mutex>& lock) {
			m_SyncAwaiters++;
			const uint32_t target = m_SyncTail;
			do {
				m_SyncCondition.wait(lock);
			} while (m_SyncHead < target);
			m_SyncAwaiters--;
			PreventSyncWraparound();
		}

		void FlushInternal() {
			if (s_Flushing) {
				return;
			}

			s_Flushing = true;
			std::unique_lock<std::mutex> lock(m_Mutex);

			if (m_FlushReadPtr != 0) {
				lock.unlock();
				Sync();
				s_Flushing = false;
				return;
			}

			alignas(uint64_t) uint8_t commandLocalMem[MAX_COMMAND_SIZE];

			while (m_FlushReadPtr < m_CommandMem.size()) {
				uint64_t commandSize = 0;
				std::memcpy(&commandSize, &m_CommandMem[m_FlushReadPtr], sizeof(uint64_t));
				m_FlushReadPtr += sizeof(uint64_t);

				HZ_CORE_ASSERT(commandSize <= MAX_COMMAND_SIZE, "Command size exceeds MAX_COMMAND_SIZE");

				CommandBase* commandOriginal = reinterpret_cast<CommandBase*>(&m_CommandMem[m_FlushReadPtr]);
				CommandBase* commandLocal = reinterpret_cast<CommandBase*>(commandLocalMem);

				std::memcpy(commandLocalMem, reinterpret_cast<void*>(commandOriginal), commandSize);

				lock.unlock();
				commandLocal->Execute();
				lock.lock();

				if (commandLocal->sync) {
					m_SyncHead++;
					lock.unlock();
					m_SyncCondition.notify_all();
					lock.lock();
				}

				commandLocal->Destroy();
				m_FlushReadPtr += commandSize;
			}

			m_CommandMem.clear();
			m_Pending.store(false);
			m_FlushReadPtr = 0;
			PreventSyncWraparound();

			s_Flushing = false;
		}

		void NoOp() {}

		static constexpr uint64_t AlignTo8(uint64_t value) {
			return (value + 8ULL - 1ULL) & ~(8ULL - 1ULL);
		}

	public:
		CommandQueue() {
			m_CommandMem.reserve(DEFAULT_COMMAND_MEM_SIZE_KB * 1024ULL);
		}

		template<typename T, typename M, typename... Args>
		void Push(T* pInstance, M pMethod, Args&&... pArgs) {
			using CommandType = Command<T, M, false, Args...>;
			static_assert(sizeof(CommandType) <= MAX_COMMAND_SIZE, "CommandType exceeds MAX_COMMAND_SIZE");
			PushInternal<CommandType, false>(pInstance, pMethod, std::forward<Args>(pArgs)...);
		}

		template<typename T, typename M, typename... Args>
		void PushAndSync(T* pInstance, M pMethod, Args&&... pArgs) {
			using CommandType = Command<T, M, true, Args...>;
			static_assert(sizeof(CommandType) <= MAX_COMMAND_SIZE, "CommandType exceeds MAX_COMMAND_SIZE");
			PushInternal<CommandType, true>(pInstance, pMethod, std::forward<Args>(pArgs)...);
		}

		template<typename T, typename M, typename R, typename... Args>
		void PushAndRet(T* pInstance, M pMethod, R* rRet, Args&&... pArgs) {
			using CommandType = CommandRet<T, M, R, Args...>;
			static_assert(sizeof(CommandType) <= MAX_COMMAND_SIZE, "CommandType exceeds MAX_COMMAND_SIZE");
			PushInternal<CommandType, true>(pInstance, pMethod, rRet, std::forward<Args>(pArgs)...);
		}

		void FlushIfPending() {
			if (m_Pending.load()) {
				FlushInternal();
			}
		}

		void FlushAll() {
			FlushInternal();
		}

		void Sync() {
			PushAndSync(this, &CommandQueue::NoOp);
		}

		bool WaitForPending(uint32_t timeoutMs = 1) {
			if (m_Pending.load()) {
				return true;
			}

			std::unique_lock<std::mutex> lock(m_Mutex);
			if (m_Pending.load()) {
				return true;
			}

			return m_PendingCondition.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() {
				return m_Pending.load();
			});
		}

		template<typename T, typename M, typename... Args>
		void push(T* pInstance, M pMethod, Args&&... pArgs) {
			Push(pInstance, pMethod, std::forward<Args>(pArgs)...);
		}

		template<typename T, typename M, typename... Args>
		void push_and_sync(T* pInstance, M pMethod, Args&&... pArgs) {
			PushAndSync(pInstance, pMethod, std::forward<Args>(pArgs)...);
		}

		template<typename T, typename M, typename R, typename... Args>
		void push_and_ret(T* pInstance, M pMethod, R* rRet, Args&&... pArgs) {
			PushAndRet(pInstance, pMethod, rRet, std::forward<Args>(pArgs)...);
		}

		void flush_if_pending() {
			FlushIfPending();
		}

		void flush_all() {
			FlushAll();
		}

		void sync() {
			Sync();
		}

		bool wait_for_pending(uint32_t timeout_ms = 1) {
			return WaitForPending(timeout_ms);
		}

	private:
		inline static thread_local bool s_Flushing = false;

			std::mutex m_Mutex;
			std::vector<uint8_t> m_CommandMem;
			std::condition_variable m_SyncCondition;
			std::condition_variable m_PendingCondition;

		uint32_t m_SyncHead = 0;
		uint32_t m_SyncTail = 0;
		uint32_t m_SyncAwaiters = 0;
		uint64_t m_FlushReadPtr = 0;
		std::atomic<bool> m_Pending = false;
	};

}
