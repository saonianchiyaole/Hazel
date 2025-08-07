#pragma once

#include "Hazel/Renderer/Buffer.h"

#include <vulkan/vulkan.h>

namespace Hazel {


	namespace Utils {





	}

	class VulkanDescriptor {


	public:

		VulkanDescriptor();
		~VulkanDescriptor();


	private:

		VkDescriptorSetLayout  m_DescriptorSetLayout;	


	};


}