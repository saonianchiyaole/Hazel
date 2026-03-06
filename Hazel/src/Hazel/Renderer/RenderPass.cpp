#include "hzpch.h"
#include "RenderPass.h"

#include "Hazel/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/OpenGL/OpenGLRenderPass.h"

#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel {
	
	
	RenderPass::RenderPass(const RenderPassSpecification& spec) : m_Specification(spec)
	{
		
	}



	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{

		switch (RendererAPI::GetAPI()) {

		case RendererAPI::API::Vulkan:
			return MakeRef<VulkanRenderPass>(spec);
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLRenderPass>(spec);
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "Invalid Renderer API!");
		}
		
	}

}