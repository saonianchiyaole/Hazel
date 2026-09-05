#pragma once

#include "Hazel/Renderer/RenderPass.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/DescriptorSetManager.h"


namespace Hazel {


	namespace Utils {


		ByteKey GetRenderPassByteKey(const RenderPassInfo& spec);

	}

	class VulkanRenderPass{

	public:

		VulkanRenderPass(const RenderPassInfo& specification);

		inline	VkRenderPass GetRawRenderPass()	const { return m_RenderPass; }
		
	private:
		
		static VkRenderPass CreateRawRenderPass(const RenderPassInfo& spec);

	private:

		VkRenderPass m_RenderPass;

	};

}
