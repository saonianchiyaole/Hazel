#include "hzpch.h"


#include "Platform/Vulkan/VulkanDevice.h"

#include "Platform/Vulkan/VulkanContext.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include <set>

namespace Hazel {

	std::vector<const char*> VulkanPhysicalDevice::s_DeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};


	namespace Utils {

		bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {

			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;


			bool isSuitable = true;

			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			//
			{
				isSuitable = isSuitable
					&& deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
					&& deviceFeatures.geometryShader;
			}
			//Is deviceExtension suitable
			{
				isSuitable = isSuitable && CheckDeviceExtensionSupport(device);
			}
			//Is deviceExtension suitable
			{
				isSuitable = isSuitable && FindQueueFamilies(device, surface).IsComplete();
			}
			//
			{
				SwapchainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
				isSuitable = isSuitable && (!swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty());
			}
			return isSuitable;
		}
		
		//判断输入的physicalDevice 是否能满足所有需要的拓展
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {

			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(VulkanPhysicalDevice::s_DeviceExtensions.begin(), VulkanPhysicalDevice::s_DeviceExtensions.end());

			for (const auto& extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();

		}

		SwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
			SwapchainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		bool CheckDeviceExtensionSupport(Ref<VulkanPhysicalDevice> device)
		{
			return false;
		}
		
		
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			VkBool32 presentSupport = false;

			for (uint32_t i = 0; i < queueFamilyCount; i++) {
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;				
				}

				if (presentSupport)
					indices.presentFamily = i;

				if (indices.IsComplete()) {
					break;
				}
			}

			return indices;
		}

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		

			for (uint32_t i = 0; i < queueFamilyCount; i++) {
				
				if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;
					indices.presentFamily = i;
					indices.transferFamily = i;
				}
				

				if (indices.IsComplete()) {
					break;
				}
			}

			return indices;
		}


	}

	VulkanPhysicalDevice::VulkanPhysicalDevice()
	{

		VkInstance instance = VulkanContext::GetVulkanInstance();		

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		std::vector<VkPhysicalDevice> physicalDevice(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevice.data());


		

		for (const auto& device : physicalDevice) {

			
			vkGetPhysicalDeviceProperties(device, &m_Properties);

			// Only check the graphics card is discrete GPU
			if (m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				//
				m_PhysicalDevice = device;

				std::cout << "Physical device found: " << std::endl;
			
				std::cout << "Device ID : " << m_Properties.deviceID << std::endl;
				std::cout << "Device Name: " << m_Properties.deviceName << std::endl;
				std::cout << "API Version : " << m_Properties.apiVersion << std::endl;
				std::cout << "Driver Version : " << m_Properties.driverVersion << std::endl;

				m_QueueFamilyIndices = Utils::FindQueueFamilies(m_PhysicalDevice);

				break;
			}
		}
	



	}

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{
	}


	//-----------------------------------------------Vulkan Device---------------------------------------------------------
	
	
	VulkanDevice::VulkanDevice(Ref<VulkanPhysicalDevice> physicalDevice) : m_PhysicalDevice(physicalDevice){


			
		QueueFamilyIndices indices = physicalDevice->GetQueueFamilyIndices();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (auto queuefamily : uniqueQueueFamilies) {

			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queuefamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);

		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = queueCreateInfos.size();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.ppEnabledExtensionNames = VulkanPhysicalDevice::s_DeviceExtensions.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(VulkanPhysicalDevice::s_DeviceExtensions.size());

		bool IsValidationLayerEnabled = VulkanContext::IsValidationLayerEnabled();

		if (IsValidationLayerEnabled) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanContext::s_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = VulkanContext::s_ValidationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateDevice(m_PhysicalDevice->GetRawPhysicalDevice(), &createInfo, nullptr, &m_Device);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicQueue);
		vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
		
		CreateCommandPool();
	}

	void VulkanDevice::CreateCommandPool() {
		
		QueueFamilyIndices queueFamilyIndices = m_PhysicalDevice->GetQueueFamilyIndices();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}

	}

	void VulkanDevice::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		HZ_CORE_ASSERT(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");

	}

	Ref<VulkanCommandBuffer> VulkanDevice::CreateCommandBuffer() {

		Ref<VulkanCommandBuffer> commandBuffer = MakeRef<VulkanCommandBuffer>(m_CommandPool);
		return commandBuffer;
	}

	Ref<VulkanCommandBuffer> VulkanDevice::CreateSecondaryCommandBuffer() {

		Ref<VulkanCommandBuffer> commandBuffer = MakeRef<VulkanCommandBuffer>(m_CommandPool, false);
		return commandBuffer;
	}

}