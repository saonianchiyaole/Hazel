#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "Hazel/Core/Core.h"

namespace Hazel {

	class ThreadPool {
	public:
		ThreadPool() = default;
		ThreadPool(uint32_t workerCount, const std::string& workerNamePrefix = "Worker");
		~ThreadPool();

		void Start(uint32_t workerCount, const std::string& workerNamePrefix = "Worker");
		void Stop();

		bool IsRunning() const { return m_Running.load(); }
		uint32_t GetWorkerCount() const { return static_cast<uint32_t>(m_Workers.size()); }

		template<typename FuncT, typename... Args>
		auto Enqueue(FuncT&& func, Args&&... args) -> std::future<std::invoke_result_t<FuncT, Args...>>
		{
			using ReturnType = std::invoke_result_t<FuncT, Args...>;

			auto task = std::make_shared<std::packaged_task<ReturnType()>>(
				std::bind(std::forward<FuncT>(func), std::forward<Args>(args)...)
			);

			std::future<ReturnType> result = task->get_future();
			{
				std::lock_guard<std::mutex> lock(m_TaskMutex);
				HZ_CORE_ASSERT(m_Running.load(), "ThreadPool is not running");
				m_Tasks.emplace([task]() { (*task)(); });
			}

			m_TaskCondition.notify_one();
			return result;
		}

	private:
		void WorkerLoop(uint32_t workerIndex, const std::string& workerNamePrefix);

	private:
		std::vector<std::thread> m_Workers;
		std::queue<std::function<void()>> m_Tasks;

		std::mutex m_TaskMutex;
		std::condition_variable m_TaskCondition;

		std::atomic<bool> m_Running = false;
		bool m_StopRequested = false;
	};

}
