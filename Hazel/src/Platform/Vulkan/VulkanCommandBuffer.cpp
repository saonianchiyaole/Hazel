#include "hzpch.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Hazel {


	



	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer commandBuffer) : m_CommandBuffer(commandBuffer)
	{
		
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool commandPool)
	{

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(device, &allocInfo, &m_CommandBuffer);

	}

	VulkanCommandBuffer::VulkanCommandBuffer()
	{
		Ref<VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		VkCommandPool commandPool = device->GetCommandPool();
		VkDevice rawDevice = device->GetRawDevice();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(rawDevice, &allocInfo, &m_CommandBuffer);


		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		HZ_CORE_ASSERT(vkCreateFence(rawDevice, &fenceInfo, nullptr, &m_Fence) == VK_SUCCESS, "Failed to create fence!");
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{

		/*Ref<VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		VkCommandPool commandPool = device->GetCommandPool();
		VkDevice rawDevice = device->GetRawDevice();

		vkFreeCommandBuffers(rawDevice, commandPool, 1, &m_CommandBuffer);*/

	}

	void VulkanCommandBuffer::Begin()
	{

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

	}


	void VulkanCommandBuffer::Submit()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		VkQueue graphicsQueue = VulkanContext::GetCurrentContext()->GetDevice()->GetGraphicQueue();

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_Fence);
		vkQueueWaitIdle(graphicsQueue);
	}



	void VulkanCommandBuffer::End()
	{



		// todo : hard code
		
		HZ_CORE_ASSERT(vkEndCommandBuffer(m_CommandBuffer) == VK_SUCCESS, "Failed to end command buffer");


		

	}

}

