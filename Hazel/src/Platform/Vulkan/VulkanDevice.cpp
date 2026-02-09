#include "hzpch.h"


#include "Platform/Vulkan/VulkanDevice.h"

#include "Platform/Vulkan/VulkanContext.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include "Hazel/Renderer/Renderer.h"

#include <set>

namespace Hazel {

	std::vector<const char*> VulkanPhysicalDevice::s_DeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	static const int MAX_UNIVERSAL_QUEUE_COUNT = 4;


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
				indices.familys[queueFamilies[i].queueFlags] = {i, queueFamilies[i].queueCount};

				if (!indices.presentFamily.has_value() && presentSupport)
					indices.presentFamily = i;
				
			}

			return indices;
		}		

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			VkBool32 presentSupport = false;

			for (uint32_t i = 0; i < queueFamilyCount; i++) {
				
				indices.familys[queueFamilies[i].queueFlags] = { i, queueFamilies[i].queueCount };
				
			}
									
			
			return indices;
		}

		bool PickPresentFamilyIndex(QueueFamilyIndices& indices, VkPhysicalDevice device, VkSurfaceKHR surface) {

			VkBool32 presentSupport = false;

			for (auto& [flagBit, pair] : indices.familys) {

				auto& [familyIndex, count] = pair;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, familyIndex, surface, &presentSupport);			

				if (!indices.presentFamily.has_value() && presentSupport){
					indices.presentFamily = familyIndex;
					return true;
				}

			}

			return false;

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

		
			
		QueueFamilyIndices& indices = physicalDevice->GetQueueFamilyIndices();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;		

		// todo make it more flexible		
		
		SelectQueue(indices);
		
		
		float queuePriority = 1.0f;
		
		std::set<uint32_t> queueFamily = { indices.computeFamily.value(), indices.transferFamily.value(), indices.graphicsFamily.value()};

		for (auto queueFamilyIndex : queueFamily) {
			
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
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
		vkGetDeviceQueue(m_Device, indices.transferFamily.value(), 0, &m_TransferQueue);
		vkGetDeviceQueue(m_Device, indices.computeFamily.value(), 0, &m_ComputeQueue);				
			
		CreateCommandPool();
	}

	void VulkanDevice::CreateCommandPool() {
		
		const QueueFamilyIndices& queueFamilyIndices = m_PhysicalDevice->GetQueueFamilyIndices();

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
		uint32_t frameInFlight = Renderer::GetFrameInFlight();

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = static_cast<uint32_t>(frameInFlight);
		
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		poolInfo.maxSets = static_cast<uint32_t>(frameInFlight);

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

	void VulkanDevice::SelectQueue(QueueFamilyIndices& indices)
	{


		bool hasUniversalQueueFamily = false;
		uint32_t universalQueueFamilyIndex = 0;

		m_SupportAsyncCompute = false;
		m_SupportAsyncTransfer = false;

		bool hasUniqueGraphicQueueFamily = false;

		for (auto& [flagBit, pair] : indices.familys) {

			auto [familyIndex, count] = pair;

			if (!hasUniversalQueueFamily && flagBit & (QueueFlagBits::QUEUE_GRAPHICS_BIT | QueueFlagBits::QUEUE_GRAPHICS_BIT | QueueFlagBits::QUEUE_TRANSFER_BIT)) {
				hasUniversalQueueFamily = true;
				universalQueueFamilyIndex = familyIndex;
			}

			if (!m_SupportAsyncCompute && flagBit & QueueFlagBits::QUEUE_COMPUTE_BIT && flagBit & QueueFlagBits::QUEUE_TRANSFER_BIT && (flagBit & QueueFlagBits::QUEUE_GRAPHICS_BIT) == 0) {
				m_SupportAsyncCompute = true;
				indices.computeFamily = familyIndex;
			}

			if (!m_SupportAsyncTransfer && flagBit & QueueFlagBits::QUEUE_TRANSFER_BIT && (flagBit & (QueueFlagBits::QUEUE_COMPUTE_BIT | QueueFlagBits::QUEUE_GRAPHICS_BIT)) == 0) {
				m_SupportAsyncTransfer = true;
				indices.transferFamily = familyIndex;
			}

			if (!hasUniqueGraphicQueueFamily && flagBit & (QueueFlagBits::QUEUE_GRAPHICS_BIT) && flagBit & QueueFlagBits::QUEUE_TRANSFER_BIT && (flagBit & QueueFlagBits::QUEUE_COMPUTE_BIT) == 0) {
				indices.graphicsFamily = familyIndex;
			}			
		}


		if (hasUniversalQueueFamily && !indices.graphicsFamily.has_value()) {
			indices.graphicsFamily = universalQueueFamilyIndex;
		}

		if (hasUniversalQueueFamily && !indices.transferFamily.has_value()) {
			indices.transferFamily = universalQueueFamilyIndex;
		}

		if (hasUniversalQueueFamily && !indices.computeFamily.has_value()) {
			indices.computeFamily = universalQueueFamilyIndex;
		}
	
	}

	void VulkanDevice::CreatePresentQueue(const QueueFamilyIndices& indices){

		vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);

	}

}