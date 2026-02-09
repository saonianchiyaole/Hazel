#pragma once

#include <thread>
#include <atomic>
#include <mutex>

#include "Hazel/Async/CommandQueue.h"

namespace Hazel {

	class Application;

	class RenderThread {

	public:
		RenderThread();

		void Start(Application* app);

		void Run();

		void Stop();
		
		template<typename FuncT, typename ...Args>
		inline void Submit(FuncT&& func, Args&&... args) {

			m_CommandQueue.Enqueue(std::forward<FuncT>(func), std::forward<Args>(args)...);

		}

	private:

		

		std::mutex m_Mutex;
 
		std::thread m_Thread;

		Application* app;

		CommandQueue m_CommandQueue;
		
	};

}