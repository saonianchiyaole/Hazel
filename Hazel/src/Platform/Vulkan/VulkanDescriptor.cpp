#include "hzpch.h"


#include "Platform/Vulkan/VulkanDescriptor.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Hazel {


	namespace Utils {




	}


	VulkanDescriptor::VulkanDescriptor(){


		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional, used for sampler bindings

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		
		HZ_CORE_ASSERT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayout) == VK_SUCCESS, "Failed to create descriptor!");
	
	}

	VulkanDescriptor::~VulkanDescriptor()
	{

	}

}
