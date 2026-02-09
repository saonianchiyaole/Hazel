#include "hzpch.h"

#include "Hazel/Async/RenderThread.h"
#include "Hazel/Async/Thread.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Core/Application.h"

namespace Hazel {


	RenderThread::RenderThread() {
		

	}

	void RenderThread::Start(Application* app)
	{
		
		m_Thread = std::thread(&RenderThread::Run, this);
		
		Utils::SetThreadName(m_Thread.native_handle(), "RenderThread");

		HZ_CORE_INFO("RenderThread Start !!!");
	}

	void RenderThread::Run() {


	
		HZ_CORE_ASSERT(m_Thread.joinable(), "Thread has been start!");	


		while (Application::GetInstance().IsRunning()) {


			auto task = m_CommandQueue.Dequeue();

			task();

		}

	}
	

	void RenderThread::Stop() {


	}


}