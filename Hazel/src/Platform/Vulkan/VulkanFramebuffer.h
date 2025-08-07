#pragma once


#include "Hazel/Renderer/Framebuffer.h"

#include "vulkan/vulkan.h"



namespace Hazel {

	class VulkanRenderPass;

	class VulkanFramebuffer : public Framebuffer {



	public:


		VulkanFramebuffer(FramebufferSpecification specification);


		inline VkFramebuffer GetRawFramebuffer() const { return m_Framebuffer; }

	private:

		VkFramebuffer m_Framebuffer;

		Ref<VulkanRenderPass> m_RenderPass;

	};

}
