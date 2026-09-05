#pragma once

#include "Hazel/Core/CommandQueue.h"

#include <functional>
#include <mutex>
#include <queue>
#include <future>

namespace Hazel {

	using CommandQueueMT = CommandQueue;


	class UniversalCommandQueue {
	public:
		UniversalCommandQueue() = default;
		~UniversalCommandQueue() = default;


		template<typename FuncT, typename ...Args>
		auto Enqueue(FuncT&& func, Args&&... args) -> std::future<typename std::invoke_result_t<FuncT, Args...>> {

			
			using returnType = typename std::invoke_result_t<FuncT, Args...>;

			auto task = std::make_shared< std::packaged_task<returnType()> >(
				std::bind(std::forward<FuncT>(func), std::forward<Args>(args)...)
			);

			std::future<returnType> res = task->get_future();
			{
				std::unique_lock<std::mutex> lock(m_Mutex);

				m_Tasks.emplace([task]() {(*task)(); });
			}

			m_Condition.notify_one();
								
			
			return res;
		}


		std::function<void()> Dequeue() {
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Condition.wait(lock, [this]() { return !m_Tasks.empty(); });
			auto task = m_Tasks.front();
			m_Tasks.pop();
			return task;
		}


	private:

		std::mutex m_Mutex;

		std::queue<std::function<void()>> m_Tasks;

		std::condition_variable m_Condition;
	};

}
