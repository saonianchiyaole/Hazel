#include "hzpch.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

#include "Hazel/Renderer/Renderer.h"

#include "vulkan/vulkan.h"

#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanPipeline.h"


namespace Hazel {

		
	VulkanRenderPass::VulkanRenderPass(const RenderPassInfo& specification) {
				
		Ref<VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		VkDevice rawDevice = device->GetRawDevice();
		
		m_RenderPass = VulkanRenderPass::CreateRawRenderPass(specification);						
	}
	
	

	VkRenderPass VulkanRenderPass::CreateRawRenderPass(const RenderPassInfo& spec)
	{

		Ref<VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		VkRenderPass vkRenderPass = nullptr;

		std::vector<VkAttachmentDescription> attachmentDescriptions;
		std::vector<VkAttachmentReference> colorAttachmentRefs;

		VkAttachmentDescription depthAttachment{};
		VkAttachmentReference depthAttachmentRef{};
		bool hasDepth = false;

		uint32_t attachmentIndex = 0;

		for (auto attachment : spec.attachmentInfos) {

			if (Utils::IsDepthFormat(attachment.format)) {

				// force the depth attachment to be the last one :)

				depthAttachment.format = Utils::GetVulkanFormatFromTextureFormat(attachment.format);
				depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				depthAttachment.loadOp = attachment.isClearColor ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				depthAttachmentRef.attachment = spec.attachmentInfos.size() - 1;
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				hasDepth = true;

				continue;
			}


			VkAttachmentDescription& attachemntDescription = attachmentDescriptions.emplace_back();
			attachemntDescription.format = Utils::GetVulkanFormatFromTextureFormat(attachment.format);
			attachemntDescription.samples = VkSampleCountFlagBits(attachment.samples);
			
			attachemntDescription.loadOp = attachment.isClearColor ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachemntDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachemntDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachemntDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference& colorAttachmentRef = colorAttachmentRefs.emplace_back();
			colorAttachmentRef.attachment = attachmentIndex;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachmentIndex++;
		}


		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colorAttachmentRefs.size();
		subpass.pColorAttachments = colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		if (hasDepth) {
			attachmentDescriptions.emplace_back(depthAttachment);
		}

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentDescriptions.size();
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		HZ_CORE_ASSERT(vkCreateRenderPass(device->GetRawDevice(), &renderPassInfo, nullptr, &vkRenderPass) == VK_SUCCESS, "Failed to create render pass for framebuffer");

		return vkRenderPass;
		
	}	

}
