#pragma once

#include "Hazel/Core.h"
#include "Hazel/Event/Event.h"
#include "Window.h"
#include "Hazel/Event/ApplicationEvent.h"
#include "Hazel/LayerStack.h"


namespace Hazel {

	class HAZEL_API Application
	{

	public :
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static inline Application& GetInstance() {
			if (s_Instance == nullptr)
				s_Instance = new Application;
			return *s_Instance;
		}

		inline Window& GetWindow() { return *m_Window; };
	private:

		bool OnWindowClose(WindowCloseEvent& e);
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		
		static Application* s_Instance;

		LayerStack m_LayerStack;

	};
	// To be define in Client
	Application* CreateApplication();
}

