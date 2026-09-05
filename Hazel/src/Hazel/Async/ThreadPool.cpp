#include "hzpch.h"

#include "Hazel/Async/ThreadPool.h"
#include "Hazel/Async/Thread.h"

namespace Hazel {

	ThreadPool::ThreadPool(uint32_t workerCount, const std::string& workerNamePrefix)
	{
		Start(workerCount, workerNamePrefix);
	}

	ThreadPool::~ThreadPool()
	{
		Stop();
	}

	void ThreadPool::Start(uint32_t workerCount, const std::string& workerNamePrefix)
	{
		if (m_Running.load()) {
			return;
		}

		if (workerCount == 0) {
			workerCount = 1;
		}

		{
			std::lock_guard<std::mutex> lock(m_TaskMutex);
			m_StopRequested = false;
		}

		m_Workers.reserve(workerCount);
		m_Running.store(true);
		for (uint32_t i = 0; i < workerCount; i++) {
			m_Workers.emplace_back([this, i, workerNamePrefix]() {
				WorkerLoop(i, workerNamePrefix);
			});
		}
	}

	void ThreadPool::Stop()
	{
		if (!m_Running.load()) {
			return;
		}

		{
			std::lock_guard<std::mutex> lock(m_TaskMutex);
			m_StopRequested = true;
		}

		m_TaskCondition.notify_all();

		for (auto& worker : m_Workers) {
			if (worker.joinable()) {
				worker.join();
			}
		}

		m_Workers.clear();
		m_Running.store(false);
	}

	void ThreadPool::WorkerLoop(uint32_t workerIndex, const std::string& workerNamePrefix)
	{
		Utils::SetCurrentThreadName(workerNamePrefix + "-" + std::to_string(workerIndex));

		while (true) {
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(m_TaskMutex);
				m_TaskCondition.wait(lock, [this]() {
					return m_StopRequested || !m_Tasks.empty();
				});

				if (m_StopRequested && m_Tasks.empty()) {
					return;
				}

				task = std::move(m_Tasks.front());
				m_Tasks.pop();
			}

			task();
		}
	}

}
