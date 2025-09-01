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
		std::optional<uint32_t> transferFamily;

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
		const QueueFamilyIndices			GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }


		operator VkPhysicalDevice() {
			return m_PhysicalDevice;
		}

		//Member
	public:
		static std::vector<const char*> s_DeviceExtensions;
	private:

		VkPhysicalDeviceProperties m_Properties;
		VkPhysicalDevice m_PhysicalDevice;		
		QueueFamilyIndices m_QueueFamilyIndices;
	};


	//Logical Device
	class VulkanDevice {


	public:

		VulkanDevice() = default;
		VulkanDevice(Ref<VulkanPhysicalDevice> physicalDevice);

		//Get
		inline	VkDevice								GetRawDevice()			{ return m_Device; }
		inline	VkCommandPool							GetCommandPool()		{ return m_CommandPool; }		
		inline	VkQueue&								GetGraphicQueue()		{ return m_GraphicQueue; }
		inline	VkQueue&								GetPresentQueue()		{ return m_PresentQueue; }
		inline	Ref<VulkanPhysicalDevice>				GetPhysicalDevice()		{ return m_PhysicalDevice; }


		//Set
		inline	void	SetPhysicalDevice(Ref<VulkanPhysicalDevice> physicalDevice) { m_PhysicalDevice = physicalDevice; }

		void CreateCommandPool();
		Ref<VulkanCommandBuffer> CreateCommandBuffer();
		void CreateDescriptorPool();
		Ref<VulkanCommandBuffer> CreateSecondaryCommandBuffer();

		//void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		operator VkDevice() {
			return m_Device;
		}

	private:

	private:

		Ref<VulkanPhysicalDevice>	m_PhysicalDevice;
		VkDevice					m_Device;

		VkCommandPool							m_CommandPool;

		VkDescriptorPool m_DescriptorPool;

		VkQueue m_GraphicQueue;
		VkQueue m_PresentQueue;
						
	};

}