#pragma once


#include "vulkan/vulkan.h"



namespace Hazel {
	
	
	class VulkanPipeline;

	struct Frame
	{
		VkCommandBuffer commandBuffer;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkFence inFlightFence;
		uint32_t imageIndex;

		Ref<VulkanPipeline> pipeline;
	};



}