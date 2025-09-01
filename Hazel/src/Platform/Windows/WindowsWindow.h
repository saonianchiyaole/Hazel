#pragma once

#include "Hazel/Core/Window.h"
#include "GLFW/glfw3.h"




namespace Hazel {

	class GraphicsContext;
	class VulkanSwapchain;
	class Swapchain;

	class WindowsWindow : public Window {
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }


		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline virtual void* GetNativeWindow() const { return m_Window; }

		virtual Ref<GraphicsContext> GetGraphicsContext() override { return m_Context; }
		virtual Ref<Swapchain> GetSwapchain() const override { return m_Swapchain; }

		GLFWwindow& GetGLFWWindow() { return *m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		Ref<GraphicsContext> m_Context = nullptr;

		Ref<Swapchain> m_Swapchain = nullptr;
		

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};
		WindowData m_Data;
	};
}