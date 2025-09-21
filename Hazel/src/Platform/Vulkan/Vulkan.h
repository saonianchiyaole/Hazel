#pragma once


#include <vulkan/vulkan.h>


#include <string>



#define VK_CHECK(f)\
{\
VkResult res = (f); \
::Hazel::VulkanCheckResult(res, __FILE__, __LINE__);\
}



namespace Hazel {


	void VulkanCheckResult(VkResult result, const std::string& fileName, const std::string& line);


}