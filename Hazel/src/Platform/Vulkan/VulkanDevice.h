#pragma once



#include "vulkan/vulkan.h"

#include "Platform/Vulkan/VulkanSwapChain.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include <optional>



namespace Hazel {

	class VulkanCommandBuffer;
	class VulkanPhysicalDevice;

	typedef enum QueueFlagBits {
		QUEUE_GRAPHICS_BIT = 0x00000001,
		QUEUE_COMPUTE_BIT = 0x00000002,
		QUEUE_TRANSFER_BIT = 0x00000004,
		QUEUE_SPARSE_BINDING_BIT = 0x00000008,
		QUEUE_PROTECTED_BIT = 0x00000010,
		QUEUE_VIDEO_DECODE_BIT_KHR = 0x00000020,
		QUEUE_VIDEO_ENCODE_BIT_KHR = 0x00000040,
		QUEUE_OPTICAL_FLOW_BIT_NV = 0x00000100,
		QUEUE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	} QueueFlagBits;

	struct QueueFamilyIndices {			

		// [flagBit] [indice, count], don't include present family, 
		// and present family only select the first queue that support present as the present queue family
		std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> familys;

		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily;

		std::optional<uint32_t> presentFamily;

		bool IsComplete() const {


			// todo , for now we don't need decode part

			/*bool supportGraphics = false;
			bool supportCompute = false;
			bool supportTransfer = false;

			for (auto& [flag, pair] : familys) {

				if (flag & QueueFlagBits::QUEUE_GRAPHICS_BIT) {
					supportGraphics = true;
				}
				if (flag & QueueFlagBits::QUEUE_TRANSFER_BIT) {
					supportTransfer = true;
				}
				if (flag & QueueFlagBits::QUEUE_COMPUTE_BIT) {
					supportCompute = true;
				}
			}

			return supportGraphics && supportCompute && supportTransfer && presentFamily.has_value();*/

			return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() && transferFamily.has_value();
		}

		bool FindQueueFamily(const uint32_t queueFlagBits, uint32_t& indice, uint32_t& count) const {

			if (familys.find(queueFlagBits) == familys.end())
				return false;

			const auto& [indiceVal, countVal] = familys.at(queueFlagBits);
			indice = indiceVal;
			count = countVal;			

			return true;
		}

	};


	namespace Utils {


		bool						CheckDeviceExtensionSupport		(VkPhysicalDevice device);
		QueueFamilyIndices			FindQueueFamilies				(VkPhysicalDevice device, VkSurfaceKHR surface);
		SwapchainSupportDetails		QuerySwapChainSupport			(VkPhysicalDevice device, VkSurfaceKHR surface);		
		bool						PickPresentFamilyIndex			(QueueFamilyIndices& indices, VkPhysicalDevice device, VkSurfaceKHR surface);
	}
		
	class VulkanPhysicalDevice {

		//Function
	public:

		VulkanPhysicalDevice();
		~VulkanPhysicalDevice();



		const VkPhysicalDevice				GetRawPhysicalDevice()	const	{ return m_PhysicalDevice; }
		const VkPhysicalDeviceProperties	GetProperties()			const	{ return m_Properties; }
		QueueFamilyIndices&					GetQueueFamilyIndices()			{ return m_QueueFamilyIndices; }
		void								SetQueueFamilyIndices(const QueueFamilyIndices& indices) { m_QueueFamilyIndices = indices; }		

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
		inline	void SetPhysicalDevice(Ref<VulkanPhysicalDevice> physicalDevice) { m_PhysicalDevice = physicalDevice; }

		void CreateCommandPool();
		Ref<VulkanCommandBuffer> CreateCommandBuffer();
		void CreateDescriptorPool();
		Ref<VulkanCommandBuffer> CreateSecondaryCommandBuffer();

		//void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		operator VkDevice() {
			return m_Device;
		}

	private:

		void SelectQueue(QueueFamilyIndices& indices);

		void CreatePresentQueue(const QueueFamilyIndices& indices);

	private:

		Ref<VulkanPhysicalDevice>	m_PhysicalDevice;
		VkDevice					m_Device;

		VkCommandPool							m_CommandPool;

		VkDescriptorPool m_DescriptorPool;



		// can be used for transfer, graphics, compute
		// always use the first as the main queue for main render thread
		 		
		bool m_SupportAsyncCompute = false;
		bool m_SupportAsyncTransfer = false;


		VkQueue m_GraphicQueue;
		VkQueue m_ComputeQueue;
		VkQueue m_TransferQueue;
		VkQueue m_PresentQueue;

		friend class VulkanSwapchain;
						
	};

}