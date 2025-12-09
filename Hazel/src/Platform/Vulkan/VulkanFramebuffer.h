#pragma once


#include "Hazel/Renderer/Framebuffer.h"

#include "vulkan/vulkan.h"



namespace Hazel {
	
	class VulkanRenderPass;

	class VulkanFramebuffer : public Framebuffer {
	public:

		VulkanFramebuffer(const FramebufferSpecification& specification);

		virtual void			Invalidate() override;
		virtual void			Resize(const FramebufferSpecification& spec) override;
		virtual void			Resize(const glm::vec2 size) override;

		virtual void			Bind() {};
		virtual void			Unbind() {};

		virtual int				ReadPixel(uint32_t attachmentIndex, int x, int y) override { return 0; }
		virtual void			ClearAttachment(uint32_t attachmentIndex, const void* value) override {}
		virtual const void		BindTexture(uint32_t index, uint32_t slot = 0) {};

		inline VkFramebuffer	GetRawFramebuffer() const { return m_Framebuffer; }
		inline VkRenderPass		GetRawRenderPass() const { return m_RawRenderPass; }

	private:

		VkFramebuffer m_Framebuffer = nullptr;
		VkRenderPass m_RawRenderPass = nullptr;		
	};

}
