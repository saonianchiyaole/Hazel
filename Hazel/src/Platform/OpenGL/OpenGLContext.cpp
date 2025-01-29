#pragma once

#include "hzpch.h"
#include "Platform/OpenGL/OpenGLContext.h"


#include "GLFW/glfw3.h"
#include "glad/glad.h"

namespace Hazel {

	OpenGLContext::OpenGLContext() {

	}

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_WindowHandle(windowHandle)
	{
		HZ_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		HZ_CORE_ASSERT(status, "Failed to initialize Glad!");

		HZ_CORE_INFO("OpenGL Info : ");
		HZ_CORE_INFO(" Vendor: {0}", fmt::ptr(glGetString(GL_VENDOR)));
		HZ_CORE_INFO(" Version: {0}", fmt::ptr(glGetString(GL_VERSION)));
		HZ_CORE_INFO(" Renderer: {0}", fmt::ptr(glGetString(GL_RENDERER)));

	}

	void OpenGLContext::Swapbuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}