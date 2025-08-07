#pragma once



#include "vulkan/vulkan.h"

#include "Platform/Vulkan/VulkanSwapChain.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include <optional>



namespace Hazel {

	class VulkanCommandBuffer;
	class VulkanPhysicalDevice;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() {
			return graphicsFamily.has_value() && presentFamily;
		}
	};


	namespace Utils {


		bool						CheckDeviceExtensionSupport		(VkPhysicalDevice device);
		QueueFamilyIndices			FindQueueFamilies				(VkPhysicalDevice device, VkSurfaceKHR surface);
		SwapchainSupportDetails		QuerySwapChainSupport			(VkPhysicalDevice device, VkSurfaceKHR surface);
		QueueFamilyIndices			FindQueueFamilies				(VkPhysicalDevice device);
	}
	




	class VulkanPhysicalDevice {

		//Function
	public:

		VulkanPhysicalDevice();
		~VulkanPhysicalDevice();



		const VkPhysicalDevice				GetRawPhysicalDevice()	const { return m_PhysicalDevice; }
		const VkPhysicalDeviceProperties	GetProperties()			const { return m_Properties; }



		operator VkPhysicalDevice() {
			return m_PhysicalDevice;
		}

		//Member
	public:
		static std::vector<const char*> s_DeviceExtensions;
	private:

		VkPhysicalDeviceProperties m_Properties;
		VkPhysicalDevice m_PhysicalDevice;


	};


	//Logical Device
	class VulkanDevice {


	public:

		VulkanDevice() = default;
		VulkanDevice(Ref<VulkanPhysicalDevice> physicalDevice);

		//Get
		inline	VkDevice								GetRawDevice()			{ return m_Device; }
		inline	VkCommandPool							GetCommandPool()		{ return m_CommandPool; }
		inline	std::vector<Ref<VulkanCommandBuffer>>	GetCommandBuffers()		{ return m_CommandBuffers; }
		inline	VkQueue&								GetGraphicQueue()		{ return m_GraphicQueue; }
		inline	VkQueue&								GetPresentQueue()		{ return m_PresentQueue; }
		inline	Ref<VulkanPhysicalDevice>				GetPhysicalDevice()		{ return m_PhysicalDevice; }


		//Set
		inline	void	SetPhysicalDevice(Ref<VulkanPhysicalDevice> physicalDevice) { m_PhysicalDevice = physicalDevice; }



		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateDescriptorPool();

		//void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		operator VkDevice() {
			return m_Device;
		}

	private:

	private:

		Ref<VulkanPhysicalDevice>	m_PhysicalDevice;
		VkDevice					m_Device;

		VkCommandPool							m_CommandPool;
		std::vector<Ref<VulkanCommandBuffer>>	m_CommandBuffers;

		VkDescriptorPool m_DescriptorPool;

		VkQueue m_GraphicQueue;
		VkQueue m_PresentQueue;

		
		


	};



}