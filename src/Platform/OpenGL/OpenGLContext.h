#pragma once

#include "Hazel/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Hazel {

	class HAZEL_API OpenGLContext : public GraphicsContext {
	public:

		OpenGLContext();
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void Swapbuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}