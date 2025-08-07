#include "hzpch.h"

#include "Hazel/Renderer/GraphicsContext.h"

#include "Hazel/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"


namespace Hazel {

	Ref<GraphicsContext> GraphicsContext::Create(GLFWwindow* window) {


		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::None:
			throw std::runtime_error("Invaild Renderer API!");
			break;
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLContext>(window);
			break;
		case RendererAPI::API::Vulkan:
			return MakeRef<VulkanContext>(window);
				break;
		default:
			throw std::runtime_error("Invaild Renderer API!");
		}
	

	}

}