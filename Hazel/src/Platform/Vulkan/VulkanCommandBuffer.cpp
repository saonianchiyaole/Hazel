#include "hzpch.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Hazel {


	



	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer commandBuffer) : m_CommandBuffer(commandBuffer)
	{
		
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool commandPool, bool isPrimary) : m_IsPrimary(isPrimary)
	{

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(device, &allocInfo, &m_CommandBuffer);

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;

		HZ_CORE_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &m_Fence) == VK_SUCCESS, "Failed to create fence!");

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
		fenceInfo.flags = 0;

		HZ_CORE_ASSERT(vkCreateFence(rawDevice, &fenceInfo, nullptr, &m_Fence) == VK_SUCCESS, "Failed to create fence!");

		m_IsPrimary = true;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{

		static const Ref<VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		VkCommandPool commandPool = device->GetCommandPool();
		VkDevice rawDevice = device->GetRawDevice();

		vkFreeCommandBuffers(rawDevice, commandPool, 1, &m_CommandBuffer);
		if(m_Fence != nullptr)
			vkDestroyFence(rawDevice, m_Fence, nullptr);

	}

	void VulkanCommandBuffer::Begin()
	{

	
		static const VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;


		vkWaitForFences(device, 1, &m_Fence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &m_Fence);

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

