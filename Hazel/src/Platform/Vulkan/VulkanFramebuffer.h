#pragma once


#include "Hazel/Renderer/Framebuffer.h"

#include "vulkan/vulkan.h"



namespace Hazel {
	
	class VulkanRenderPass;
	class VulkanTexture2D;


	enum FramebufferStage {

		Initialize,
		AfterRender,
		Output,

	};

	class VulkanFramebuffer {
	public:

		VulkanFramebuffer(const FramebufferInfo& info, const Ref<VulkanRenderPass> renderPass);
		VulkanFramebuffer(const FramebufferInfo& info, const Ref<VulkanRenderPass> renderPass, std::vector<Ref<VulkanTexture2D>> attacments);

		const FramebufferInfo&						GetSpecification() { return m_Info; }
		const std::vector<Ref<VulkanTexture2D>>		GetColorAttachments() { return m_ColorAttachments; }
		const Ref<VulkanTexture2D>					GetColorAttachment(int index = 0){
			HZ_CORE_ASSERT(index < m_ColorAttachments.size(), "Index out of range");
			return m_ColorAttachments[index];
		}	
		const Ref<VulkanTexture2D>					GetDpethAttachment() { return m_DepthAttachment; }

		void										Update(const FramebufferInfo& info, const Ref<VulkanRenderPass> renderPass, std::vector<Ref<VulkanTexture2D>> attachments);
												
		inline VkFramebuffer						GetRawFramebuffer() const { return m_Framebuffer; }

		
	private:

		FramebufferInfo m_Info;
		
		VkFramebuffer m_Framebuffer = nullptr;
		

		// strong ref 
		Ref<VulkanTexture2D>				m_DepthAttachment = nullptr;
		std::vector<Ref<VulkanTexture2D>>	m_ColorAttachments;
		
		
	};

}
