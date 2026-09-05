#pragma once


#include "Hazel/Renderer/CommandBuffer.h"
#include "vulkan/vulkan.h"

namespace Hazel {


	class VulkanCommandBuffer
	{


	public :


		VulkanCommandBuffer();		
		VulkanCommandBuffer(VkCommandPool commandPool, bool isPrimary = true);
		VulkanCommandBuffer(VkCommandBuffer commandBuffer);		
		~VulkanCommandBuffer();

		void Begin();

		void Submit();

		void End();
		
		inline	VkCommandBuffer&		GetRawCommandBuffer()		{ return m_CommandBuffer; }

	private:

		VkCommandBuffer m_CommandBuffer = nullptr;
		VkFence m_Fence;
		bool m_IsPrimary = true;
	};


}
