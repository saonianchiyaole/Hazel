#pragma once

#include "Hazel/Renderer/GraphicsContext.h"
#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"


#include "Hazel/Core/Application.h"


#include "Platform/Vulkan/VulkanSwapchain.h"


#define MAX_FRAMES_IN_FLIGHT 3


namespace Hazel {


	class VulkanPhysicalDevice;
	class VulkanDevice;
	class VulkanSwapchain;

	namespace Utils {

		VkResult						CreateDebugUtilsMessengerEXT		(VkInstance instance, 
																			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
																			const VkAllocationCallbacks* pAllocator, 
																			VkDebugUtilsMessengerEXT* pDebugMessenger);

		void							DestroyDebugUtilsMessengerEXT		(VkInstance instance, 
																			VkDebugUtilsMessengerEXT debugMessenger, 
																			const VkAllocationCallbacks* pAllocator);

		bool							CheckValidationLayerSupport			(const std::vector<const char*> validationLayers);

		std::vector<const char*>		GetRequiredExtensions				();

		void							PopulateDebugMessengerCreateInfo	(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback						(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
																			VkDebugUtilsMessageTypeFlagsEXT messageType, 
																			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
																			void* pUserData);

	}

	class VulkanContext : public GraphicsContext {

	public:
		VulkanContext(GLFWwindow* window);

		virtual void Init() override;
		virtual void Swapbuffers() override;



		inline VkSurfaceKHR					GetSurface()									{ return m_Swapchain->GetSurface(); }
		inline Ref<VulkanPhysicalDevice>	GetPhysicalDevice()								{ return m_PhysicalDevice; }
		inline Ref<VulkanDevice>			GetDevice()										{ return m_Device; }
		inline GLFWwindow*					GetWindow()										{ return m_Window; }
		inline Ref<VulkanSwapchain>			GetSwapchain()									{ return m_Swapchain; }


		static inline bool					IsValidationLayerEnabled()						{ return s_IsValidationLayersEnabled; }
		static VkInstance					GetVulkanInstance()								{ return s_VulkanInstance; }
		static Ref<VulkanContext>			GetCurrentContext()								{ return std::dynamic_pointer_cast<VulkanContext>(Application::GetInstance().GetWindow().GetGraphicsContext()); }								
		static std::vector<const char*>		GetValidationLayers()							{ return s_ValidationLayers; };

		//assume that there is only one window/swapchain need surface 		
		inline void							SetSwapchain(Ref<VulkanSwapchain> swapchain)	{ m_Swapchain = swapchain; }

		

		//todo move this to vulkan renderer
		//void								DrawFrame();

	private:

		void CreateVulkanInstance();
		void SetupDebugMessenger();		
		void PickUpPhysicalDevice();
		void CreateLogicalDevice();

	public:

		static std::vector<const char*> s_ValidationLayers;


	private:

		static bool s_IsValidationLayersEnabled;

		VkDebugUtilsMessengerEXT m_DebugMessenger;

		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		Ref<VulkanDevice> m_Device;

		Ref<VulkanSwapchain> m_Swapchain;

		GLFWwindow* m_Window;


		static VkSurfaceKHR s_Surface;
		static VkInstance s_VulkanInstance;
				
	};



}