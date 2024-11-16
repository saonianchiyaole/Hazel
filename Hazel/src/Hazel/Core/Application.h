#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Event/Event.h"
#include "Window.h"
#include "Hazel/Event/ApplicationEvent.h"
#include "Hazel/Core/LayerStack.h"
#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Core/Timestep.h"

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
		bool OnWindowResize(WindowResizeEvent& e);
	
	private:
		
		Scope<Window> m_Window;
	
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		
		static Application* s_Instance;

		Timestep m_Timestep;
		float m_LastFrameTime = 0.0f;

		LayerStack m_LayerStack;

		bool m_Minimized = false;
	};
	// To be define in Client
	Application* CreateApplication();
}

