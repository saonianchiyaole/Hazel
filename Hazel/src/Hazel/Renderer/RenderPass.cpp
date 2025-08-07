#include "hzpch.h"
#include "RenderPass.h"

#include "Hazel/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

namespace Hazel {



	RenderPass::RenderPass(RenderPassSpecification spec)
		: m_Specification(spec)
	{
	}

	RenderPassSpecification& RenderPass::GetSpecification()
	{
		return m_Specification;
	}

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{

		switch (RendererAPI::GetAPI()) {

		case RendererAPI::API::Vulkan:
			return MakeRef<VulkanRenderPass>(spec);
		case RendererAPI::API::OpenGL:
			return MakeRef<RenderPass>(spec);
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "Invalid Renderer API!");
		}
		
	}

}