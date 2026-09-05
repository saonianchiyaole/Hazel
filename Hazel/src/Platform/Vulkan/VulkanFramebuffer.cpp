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


	VulkanFramebuffer::VulkanFramebuffer(const FramebufferInfo& info, const Ref<VulkanRenderPass> renderPass) : m_Info(info)
	{		
	}

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferInfo& info, const Ref<VulkanRenderPass> renderPass, std::vector<Ref<VulkanTexture2D>> attacments) {

								
		Update(info, renderPass, std::move(attacments));

	}



	void VulkanFramebuffer::Update(const FramebufferInfo& info, const Ref<VulkanRenderPass> renderPass, std::vector<Ref<VulkanTexture2D>> attacments)
	{
		
		if (info.width == m_Info.width && info.height == m_Info.height) {
			return;
		}

		
		for (auto attachment : attacments) {

			const auto& attachmentInfo = attachment->GetInfo();

			HZ_CORE_ASSERT(attachmentInfo.height == info.height && attachmentInfo.width == info.width,
				"Attachments's width or height should equal framebuffer's")

		}

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		vkDeviceWaitIdle(device);

		if (m_Framebuffer != nullptr) {
			vkDestroyFramebuffer(device, m_Framebuffer, NULL);
			m_Framebuffer = nullptr;
		}

		m_ColorAttachments.clear();
		m_DepthAttachment = nullptr;


		for (auto attachment : attacments) {

			const auto& attachmentInfo = attachment->GetInfo();

			if (Utils::IsDepthFormat(attachmentInfo.format)) {

				m_DepthAttachment = attachment;

				continue;
			}

			m_ColorAttachments.emplace_back(MakeRef<VulkanTexture2D>(attachmentInfo.format, m_Info.width, m_Info.height, TextureUsage::Attachment));
			
		}
	
		RenderPassInfo rdpSpec;
		rdpSpec.attachmentInfos = m_Info.attachments;

		auto vkRenderPass = renderPass->GetRawRenderPass();

		// cast		
		std::vector<Ref<VulkanTexture2D>> vulkanAttachments = m_ColorAttachments;

		std::vector<VkImageView> attachmentImageViews;
		attachmentImageViews.reserve(vulkanAttachments.size() + (m_DepthAttachment == nullptr ? 0 : 1));
		for (int i = 0; i < vulkanAttachments.size(); i++) {
			attachmentImageViews.push_back(vulkanAttachments[i]->GetImageView());
		}
		if (m_DepthAttachment) {
			attachmentImageViews.push_back(m_DepthAttachment->GetImageView());
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vkRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentImageViews.size());
		framebufferInfo.pAttachments = attachmentImageViews.data();
		framebufferInfo.width = m_Info.width;
		framebufferInfo.height = m_Info.height;
		framebufferInfo.layers = 1;



		HZ_CORE_ASSERT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffer) == VK_SUCCESS, "Failed to create framebuffer");


		m_Info = info;

	}
				

}
