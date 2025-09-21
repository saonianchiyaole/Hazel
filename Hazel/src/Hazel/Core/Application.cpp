#include "hzpch.h"
#include "Application.h"


#include "GLFW/glfw3.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Core/Window.h"
#include "Hazel/Renderer/Swapchain.h"

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace Hazel {

	Application* Application::s_Instance = nullptr;

	Application::Application(std::string windowName) {
		
		HZ_CORE_ASSERT(!s_Instance, "Application already exists");
		s_Instance = this;


		WindowProps props;
		props.Title = windowName;

		m_Window = Scope<Window>(Window::Create(props));

		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();
		ScriptEngine::Init();

		m_ImGuiLayer = ImGuiLayer::Create();

		PushLayer(m_ImGuiLayer.get());
	}

	Application::~Application() {

	}

	void Application::Run() {

		static uint32_t frame = 0;

		while (m_Running) {

			
			m_Window->GetSwapchain()->BeginFrame();

			float time = (float)glfwGetTime();
			Timestep ts = time - m_LastFrameTime;
			m_LastFrameTime = time;
			
			if(!m_Minimized)
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(ts);

			m_ImGuiLayer->Begin();

			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();

			m_Window->OnUpdate();

			frame++;
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
		//dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{

			(*--it)->OnEvent(e);

			if (e.m_Handled)
				break;
		}


	}

	void Application::Close()
	{
		m_Running = false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;

	}

}
