#pragma once

#include "Hazel/Renderer/RenderPass.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanPipeline.h"


namespace Hazel {

	

	class VulkanRenderPass : public RenderPass {

	public:


		VulkanRenderPass(const RenderPassSpecification& specification);


		inline		VkRenderPass			GetRawRenderPass()	const	{ return m_RenderPass; }
		
		inline		Ref<VulkanPipeline>		GetPipeline()				{ return std::dynamic_pointer_cast<VulkanPipeline>(m_Specification.pipeline); }

		operator	VkRenderPass()								const	{ return m_RenderPass; }
 
	private:

		VkRenderPass m_RenderPass;			 		

	};

}
