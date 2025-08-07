#pragma once

struct GLFWwindow;

namespace Hazel {

	
	class HAZEL_API GraphicsContext {
	public:
		virtual void Init() = 0;
		virtual void Swapbuffers() = 0;

		static Ref<GraphicsContext> Create(GLFWwindow* window);
	

	};

}