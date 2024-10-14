#include "hzpch.h"
#include "Application.h"


#include "Hazel/Event/ApplicationEvent.h"
#include "Hazel/Event/KeyEvent.h"

#include "glad/glad.h"
#include "Hazel/Input.h"



#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace Hazel {

	Application* Application::s_Instance = nullptr;

	Application::Application() {
		HZ_CORE_ASSERT(!s_Instance, "Application already exists");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		unsigned int id;
		
		glGenVertexArrays(1, &id);

	}

	Application::~Application() {

	}

	void Application::Run() {
		while (m_Running) {

			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);


			
			for (Layer* layer : m_LayerStack) {
				layer->OnUpdate();
			}
			
			
			m_Window->OnUpdate();
			
		}
	}


	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::OnEvent(Event& e) {

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

		
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			
			(*--it)->OnEvent(e);

			if(e.m_Handled)
				break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	// To be define in Client
	/*Application* Application::CreateApplication() {
		Application* app = new Application;
		return app;
	}*/
}
