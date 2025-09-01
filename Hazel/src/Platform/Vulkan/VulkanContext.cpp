#include "hzpch.h"


#include "Platform/Vulkan/VulkanContext.h"




#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace Hazel {


	
	std::vector<const char*> VulkanContext::s_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
	};

	VkInstance VulkanContext::s_VulkanInstance = VK_NULL_HANDLE;
	VkSurfaceKHR VulkanContext::s_Surface = VK_NULL_HANDLE;
	bool VulkanContext::s_IsValidationLayersEnabled = true;

	namespace Utils {


		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) {
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else {
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) {
				func(instance, debugMessenger, pAllocator);
			}
		}

		bool CheckValidationLayerSupport(const std::vector<const char*> validationLayers) {

			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validationLayers) {
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layerName, layerProperties.layerName) == 0) {
						layerFound = true;
						break;
					}
				}

				if (!layerFound) {
					return false;
				}
			}


			return true;
		}

		std::vector<const char*> GetRequiredExtensions() {

			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			return extensions;

		}


		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debugCallback;
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}

	}


	VulkanContext::VulkanContext(GLFWwindow* window) : m_Window(window)
	{
	}


	
	void VulkanContext::Init()
	{

		CreateVulkanInstance();

		SetupDebugMessenger();

		PickUpPhysicalDevice();

		CreateLogicalDevice();					

	}

	void VulkanContext::Swapbuffers()
	{

		m_Swapchain->Present();

		//HZ_CORE_ASSERT(false, "Function: {0} not impelemented yet! In file : {1}!", __FILE__, __FUNCTION__);
	}


	void VulkanContext::CreateVulkanInstance() {
		if (s_IsValidationLayersEnabled && !Utils::CheckValidationLayerSupport(s_ValidationLayers)) {
			throw std::runtime_error("validation layers requested, but not available!");
		}


		VkApplicationInfo appInfo = {};
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;


		auto extensions = Utils::GetRequiredExtensions();



		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();


		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		if (s_IsValidationLayersEnabled) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = s_ValidationLayers.data();

			Utils::PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}



		VkResult result = vkCreateInstance(&createInfo, nullptr, &s_VulkanInstance);

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
		

	}

	void VulkanContext::SetupDebugMessenger() {

		if (!s_IsValidationLayersEnabled) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		Utils::PopulateDebugMessengerCreateInfo(createInfo);


		VkResult result = Utils::CreateDebugUtilsMessengerEXT(GetVulkanInstance(), &createInfo, nullptr, &m_DebugMessenger);

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	
	}	


	void VulkanContext::PickUpPhysicalDevice() {


		m_PhysicalDevice = MakeRef<VulkanPhysicalDevice>();

		
	}

    void VulkanContext::CreateLogicalDevice() {  

        m_Device = MakeRef<VulkanDevice>(m_PhysicalDevice);  
		

    }


		

}



