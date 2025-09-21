#include "hzpch.h"

#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

namespace Hazel {


	namespace Utils {

		

	}


	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specification){
		
		m_Specification = specification;
		
		Resize(glm::vec2{m_Specification.width, m_Specification.height});

	}



	void VulkanFramebuffer::Invalidate()
	{

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		std::vector<VkAttachmentReference> colorAttachmentRefs;

		m_ColorAttachments.clear();
		m_DepthAttachment = nullptr;

		uint32_t attachmentIndex = 0;
		
		VkAttachmentDescription depthAttachment{};
		VkAttachmentReference depthAttachmentRef{};

		for(auto attachment : m_Specification.attachments.attachments) {

			if (Utils::IsDepthFormat(attachment.textureFormat)) {

				m_DepthAttachment = Texture2D::Create(attachment.textureFormat, m_Specification.width, m_Specification.height, TextureUsage::Attachment);
				
				// force the depth attachment to be the last one :)

				depthAttachment.format = Utils::GetVulkanFormatFromTextureFormat(attachment.textureFormat);
				depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				
				depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				
				depthAttachmentRef.attachment = m_Specification.attachments.attachments.size() - 1;
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				continue;
			}
			
			m_ColorAttachments.emplace_back(Texture2D::Create(attachment.textureFormat, m_Specification.width, m_Specification.height, TextureUsage::Attachment));

			VkAttachmentDescription& attachemnt = attachmentDescriptions.emplace_back();
			attachemnt.format = Utils::GetVulkanFormatFromTextureFormat(attachment.textureFormat);
			attachemnt.samples = VK_SAMPLE_COUNT_1_BIT;
			attachemnt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachemnt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachemnt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachemnt.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference& colorAttachmentRef = colorAttachmentRefs.emplace_back();
			colorAttachmentRef.attachment = attachmentIndex;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						
			attachmentIndex++;
		}


		

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colorAttachmentRefs.size();
		subpass.pColorAttachments = colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = m_DepthAttachment == nullptr ? nullptr : &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		if (m_DepthAttachment != nullptr) {
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
		
		HZ_CORE_ASSERT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RawRenderPass) == VK_SUCCESS, "Failed to create render pass for framebuffer");

		// cast		
		std::vector<Ref<VulkanTexture2D>> vulkanAttachments = RefVectorStaticCast<VulkanTexture2D>(m_ColorAttachments);				
		
		std::vector<VkImageView> attachmentImageViews;
		attachmentImageViews.reserve(vulkanAttachments.size() + (m_DepthAttachment == nullptr ? 0 : 1));
		for (int i = 0; i < vulkanAttachments.size(); i++) {
			attachmentImageViews.push_back(vulkanAttachments[i]->GetImageView());
		}
		if (m_DepthAttachment) {
			attachmentImageViews.push_back(std::static_pointer_cast<VulkanTexture2D>(m_DepthAttachment)->GetImageView());
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RawRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentImageViews.size());
		framebufferInfo.pAttachments = attachmentImageViews.data();
		framebufferInfo.width = m_Specification.width;
		framebufferInfo.height = m_Specification.height;
		framebufferInfo.layers = 1;

		

		HZ_CORE_ASSERT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffer) == VK_SUCCESS, "Failed to create framebuffer");
	}

	void VulkanFramebuffer::Resize(const FramebufferSpecification& spec)
	{
	}

	void VulkanFramebuffer::Resize(const glm::vec2 size)
	{
		m_Specification.width = size.x;
		m_Specification.height = size.y;
		Invalidate();

	}



}
