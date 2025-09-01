#include "hzpch.h"
#include "Hazel/Renderer/Swapchain.h"
#include "Hazel/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanSwapchain.h"
namespace Hazel {

	Ref<Swapchain> Swapchain::Create() {

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return nullptr;
			break;
		case RendererAPI::API::Vulkan:
			return MakeRef<VulkanSwapchain>();
		default:
			break;
		}


	}



}