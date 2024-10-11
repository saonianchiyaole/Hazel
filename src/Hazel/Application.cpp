#include "hzpch.h"
#include "Application.h"


#include "Hazel/Event/ApplicationEvent.h"
#include "Hazel/Event/KeyEvent.h"


namespace Hazel {
	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application() {

	}

	void Application::Run() {
		while (m_Running) {
			m_Window->OnUpdate();
		}
	}



	// To be define in Client
	/*Application* Application::CreateApplication() {
		Application* app = new Application;
		return app;
	}*/
}
