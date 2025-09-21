#include "hzpch.h"

#include "Hazel/Renderer/CommandBuffer.h"
#include "Hazel/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace Hazel {


	Ref<CommandBuffer> CommandBuffer::Create() {
	
		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL:
			return nullptr;
		case RendererAPI::API::Vulkan:
			return MakeRef<VulkanCommandBuffer>();
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "Invalid Render API!");
			return nullptr;
		}
	}


}