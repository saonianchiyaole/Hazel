#include "hzpch.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

#include "vulkan/vulkan.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Hazel {


	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification) {
				
		m_Specification.pipeline = specification.pipeline;

		m_Specification.targetFramebuffer = m_Specification.pipeline->GetTargetFramebuffer();
		
	}


}