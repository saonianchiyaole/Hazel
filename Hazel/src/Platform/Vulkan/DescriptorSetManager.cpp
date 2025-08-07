#include "hzpch.h"

#include "Platform/Vulkan/DescriptorSetManager.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanShader.h"

#include "Platform/Vulkan/VulkanBuffer.h"
#include "Hazel/Renderer/CommandBuffer.h"

#include "glm/glm.hpp"

#include "Hazel/Renderer/Camera.h"

namespace Hazel {



	struct UniformBufferObject {

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;

	};

	void DescriptorSetManager::Init()
	{		

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		// Create Descriptor Pool
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10 * 3; // frames in flight should partially determine this
		poolInfo.poolSizeCount = 10;
		poolInfo.pPoolSizes = poolSizes;		

		HZ_CORE_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");	
	
	}


	void DescriptorSetManager::CreateDescriptorSet()
	{

	}

	void DescriptorSetManager::AllocateDescriptorSet(Ref<VulkanShader> shader, uint32_t frameIndex)
	{																			

	}

	void DescriptorSetManager::WriteDescriptorBufferMemory()
	{		

	}
	
}



