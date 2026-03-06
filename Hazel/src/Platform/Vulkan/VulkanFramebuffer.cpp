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


	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specification) {

		m_Specification = specification;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &mRenderFinishedFence));

		Invalidate();

	}



	void VulkanFramebuffer::Invalidate()
	{
		
		m_Flag = AssetFlag::Loading;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		vkDeviceWaitIdle(device);

		if (m_Framebuffer != nullptr) {
			vkDestroyFramebuffer(device, m_Framebuffer, NULL);
			m_Framebuffer = nullptr;
		}

		m_ColorAttachments.clear();
		m_DepthAttachment = nullptr;

		for (auto attachment : m_Specification.attachments) {

			if (Utils::IsDepthFormat(attachment.format)) {

				m_DepthAttachment = Texture2D::Create(attachment.format, m_Specification.width, m_Specification.height, TextureUsage::Attachment);				

				continue;
			}

			m_ColorAttachments.emplace_back(Texture2D::Create(attachment.format, m_Specification.width, m_Specification.height, TextureUsage::Attachment));
			
		}
	
		RenderPassSpecification rdpSpec;
		rdpSpec.attachmentSpecs = m_Specification.attachments;

		m_RawRenderPass = VulkanRenderPass::GetRawRenderPass(rdpSpec);

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


		m_Flag = AssetFlag::Valid;

	}

	void VulkanFramebuffer::Resize(const FramebufferSpecification& spec)
	{
		
		
	}
	

	void VulkanFramebuffer::Resize(const glm::vec2 size)
	{
		if (size.x == m_Specification.width && size.y == m_Specification.height) {
			return;
		}
		
		m_Specification.width = size.x;
		m_Specification.height = size.y;

		Invalidate();
	}

	void VulkanFramebuffer::WaitRenderFinished()
	{
		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		VK_CHECK(vkWaitForFences(device, 1, &mRenderFinishedFence, VK_TRUE, UINT64_MAX));

		VK_CHECK(vkResetFences(device, 1, &mRenderFinishedFence));
	}

	void VulkanFramebuffer::ClearAllAttachments()
	{

		Ref<VulkanCommandBuffer> cmd = std::static_pointer_cast<VulkanCommandBuffer>(CommandBuffer::Create());


		for (auto colorAttachment : m_ColorAttachments) {
			Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(colorAttachment);
			Utils::TransitionImageLayout(vulkanTexture->GetRawImage(),
				Utils::GetVulkanFormatFromTextureFormat(vulkanTexture->GetTextureFormat()),
				vulkanTexture->GetLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		if (m_DepthAttachment) {
			Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(m_DepthAttachment);
			Utils::TransitionImageLayout(vulkanTexture->GetRawImage(),
				Utils::GetVulkanFormatFromTextureFormat(vulkanTexture->GetTextureFormat()),
				vulkanTexture->GetLayout(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}

		cmd->Begin();
		VkCommandBuffer rawCmd = cmd->GetRawCommandBuffer();

		RenderPassSpecification rdpSpec;
		rdpSpec.attachmentSpecs = m_Specification.attachments;
		for(auto& attachment : rdpSpec.attachmentSpecs) {
			attachment.isClearColor = true;
		}
		Ref<VulkanRenderPass> vulkanRenderPass = std::static_pointer_cast<VulkanRenderPass>(RenderPass::Create(rdpSpec));


		std::vector<VkClearValue> clearValues;
		clearValues.reserve(m_Specification.attachments.size());
		for (auto& attachment : m_Specification.attachments) {
			if (Utils::IsDepthFormat(attachment.format)) {
				VkClearValue clearValue{};
				clearValue.depthStencil.depth = attachment.clearValue.depthStencil.depth;
				clearValue.depthStencil.stencil = attachment.clearValue.depthStencil.stencil;
				clearValues.push_back(clearValue);
			}
			else {
				VkClearValue clearValue{};
				clearValue.color = *(VkClearColorValue*)&attachment.clearValue.color;
				clearValues.push_back(clearValue);
			}
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();
		renderPassInfo.renderPass = vulkanRenderPass->GetRawRenderPass();
		renderPassInfo.framebuffer = m_Framebuffer;
		renderPassInfo.renderArea = VkRect2D{ {0, 0}, {m_Specification.width, m_Specification.height} };


		

		std::vector<VkClearAttachment> clearAttachments;

		vkCmdBeginRenderPass(rawCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(rawCmd);		

		cmd->End();

		cmd->Submit();

 		TraceLayout(AfterRender);

	}

	void VulkanFramebuffer::TraceLayout(FramebufferStage stage)
	{
		switch (stage) {

		case FramebufferStage::AfterRender:
		{
			for (Ref<Texture2D> texture : m_ColorAttachments) {
				Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(texture);
				vulkanTexture->SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			}

			Ref<VulkanTexture2D> depthAttachment = std::static_pointer_cast<VulkanTexture2D>(m_DepthAttachment);
			if (depthAttachment)
				depthAttachment->SetLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}
			break;
		case FramebufferStage::Initialize:
			break;
		case FramebufferStage::Output:

			for (Ref<Texture2D> texture : m_ColorAttachments) {
				Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(texture);
				vulkanTexture->SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}

			Ref<VulkanTexture2D> depthAttachment = std::static_pointer_cast<VulkanTexture2D>(m_DepthAttachment);
			if (depthAttachment)
				depthAttachment->SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			break;
		}

	}


}
