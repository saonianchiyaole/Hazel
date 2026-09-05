#pragma once



#include "vulkan/vulkan.h"

#include "Hazel/Renderer/RenderDevice.h"
#include "Hazel/Renderer/DeviceResourceTable.h"
#include "Hazel/Renderer/RenderPass.h"
#include "Hazel/Renderer/Pipeline.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Texture.h"

#include "Platform/Vulkan/VulkanSwapChain.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanVertexArray.h"
#include "Platform/Vulkan/VulkanPipeline.h"
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
	class VulkanDevice : public RenderDevice{
	public:

		VulkanDevice();
		VulkanDevice(Ref<VulkanPhysicalDevice> physicalDevice);
		virtual ~VulkanDevice() = default;
		
		inline  bool Init() override {};
		inline  bool Init(Ref<VulkanPhysicalDevice> physicalDevice);

		inline	void Shutdown();


		inline	VkDevice								GetRawDevice()			{ return m_Device; }
		inline	VkCommandPool							GetCommandPool()		{ return m_CommandPool; }		
		inline	VkQueue&								GetGraphicQueue()		{ return m_GraphicQueue; }
		inline	VkQueue&								GetPresentQueue()		{ return m_PresentQueue; }
		inline	Ref<VulkanPhysicalDevice>				GetPhysicalDevice()		{ return m_PhysicalDevice; }

		
		//Set
		inline	void SetPhysicalDevice(Ref<VulkanPhysicalDevice> physicalDevice) { m_PhysicalDevice = physicalDevice; }

		void						CreateCommandPool();
		Ref<VulkanCommandBuffer>	CreateCommandBufferRef();
		Ref<VulkanCommandBuffer>	CreateSecondaryCommandBufferRef();		
		

		// ------------------------------------------------------------------------------------- Proxy ---------------------------------------------------------------------------------------


		
		void						CreateVertexBuffer(Handle<VertexBuffer> handle, size_t size, std::vector<uint8_t> data = {}) override;
		
		void						CreateIndexBuffer(Handle<IndexBuffer> handle, size_t size, std::vector<uint8_t> data = {}) override;
		
		void						CreateUniformBuffer(Handle<UniformBuffer> handle, const std::string name) override;

		
		void						CreateUniformBufferSet(Handle<UniformBufferSet> handle, uint32_t amount, size_t size) override { RenderDevice::CreateUniformBufferSet(handle, amount, size); }
		void						CreateUniformBufferSet(Handle<UniformBufferSet> handle, size_t size) override { RenderDevice::CreateUniformBufferSet(handle, size); }		
		void						CreateVertexArray(Handle<VertexArray> handle) override { CreateProxy(handle); }		
		void						CreateCommandBuffer(Handle<CommandBuffer> handle) override { CreateProxy(handle); }
		
		void						CreateTexture2D(Handle<Texture2D> handle, TextureInfo textureInfo, std::vector<uint8_t> data = {}) override;
		
		/*void						CreateTextureCube(Handle<TextureCube> handle, std::vector<Ref<Texture2D>> textures) override { RenderDevice::CreateTextureCube(handle, textures); }
		void						CreateTextureCube(Handle<TextureCube> handle, const std::string& path) override { RenderDevice::CreateTextureCube(handle, path); }
		void						CreateTextureCube(Handle<TextureCube> handle, TextureFormat format, uint32_t width, uint32_t height) override { RenderDevice::CreateTextureCube(handle, format, width, height); }
		void						CreateTextureCube(Handle<TextureCube> handle) override { RenderDevice::CreateTextureCube(handle); }*/
		
		void						CreateShader(Handle<Shader> handle, const std::string& vertexSrc, const std::string& fragmentSrc) override;
		void						CreateShader(Handle<Shader> handle, const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) override;
		void						CreateShader(Handle<Shader> handle, const std::string& filepath) override;
				
		void						CreateFramebuffer(Handle<Framebuffer> handle, const FramebufferInfo info, std::vector<Handle<Texture2D>> attachments = {}) override {  CreateProxy(handle, info); }		
				
		void						CreateEnvironment(Handle<Environment> handle, const std::string& filepath) override { RenderDevice::CreateEnvironment(handle, filepath); }

		// Create Proxy
		void						CreateProxy(Handle<Texture2D> handle, TextureInfo textureInfo, std::vector<uint8_t> data = {});
		void						CreateProxy(Handle<VertexBuffer> handle, size_t size, std::vector<uint8_t> data = {});		
		void						CreateProxy(Handle<IndexBuffer> handle, size_t size, std::vector<uint8_t> data = {});		
		void						CreateProxy(Handle<UniformBuffer> handle, size_t size, std::vector<uint8_t> data = {});
		void						CreateProxy(Handle<VertexArray> handle);
		void						CreateProxy(Handle<CommandBuffer> handle, bool primary = true);
		void						CreateProxy(Handle<Framebuffer> handle, const FramebufferInfo info);
		void						CreateProxy(Handle<Shader> handle, Ref<const ShaderSnapshot> shaderSnapshot);

		// Register Proxy
		void						RegisterProxy(Handle<Texture2D> handle,		Ref<VulkanTexture2D> proxy);
		void						RegisterProxy(Handle<VertexBuffer> handle,	Ref<VulkanVertexBuffer> proxy);
		void						RegisterProxy(Handle<IndexBuffer> handle,	Ref<VulkanIndexBuffer> proxy);
		void						RegisterProxy(Handle<UniformBuffer> handle,	Ref<VulkanUniformBuffer> proxy);
		void						RegisterProxy(Handle<VertexArray> handle,	Ref<VulkanVertexArray> proxy);
		void						RegisterProxy(Handle<CommandBuffer> handle,	Ref<VulkanCommandBuffer> proxy);
		void						RegisterProxy(Handle<Framebuffer> handle,	Ref<VulkanFramebuffer> proxy);
		void						RegisterProxy(Handle<Shader> handle,		Ref<VulkanShader> proxy);

		// Get Proxy
		Ref<VulkanTexture2D>		GetProxy(Handle<Texture2D> handle);
		Ref<VulkanVertexBuffer>		GetProxy(Handle<VertexBuffer> handle);
		Ref<VulkanIndexBuffer>		GetProxy(Handle<IndexBuffer> handle);
		Ref<VulkanUniformBuffer>	GetProxy(Handle<UniformBuffer> handle);
		Ref<VulkanVertexArray>		GetProxy(Handle<VertexArray> handle);
		Ref<VulkanCommandBuffer>	GetProxy(Handle<CommandBuffer> handle);
		Ref<VulkanFramebuffer>		GetProxy(Handle<Framebuffer> handle);
		Ref<VulkanShader>			GetProxy(Handle<Shader> handle);

		// Has Proxy
		bool						HasProxy(Handle<Texture2D> handle) const;
		bool						HasProxy(Handle<VertexBuffer> handle) const;
		bool						HasProxy(Handle<IndexBuffer> handle) const;
		bool						HasProxy(Handle<UniformBuffer> handle) const;
		bool						HasProxy(Handle<VertexArray> handle) const;
		bool						HasProxy(Handle<CommandBuffer> handle) const;
		bool						HasProxy(Handle<Framebuffer> handle) const;
		bool						HasProxy(Handle<Shader> handle) const;

		// Destroy Proxy
		void						DestroyProxy(Handle<Texture2D> handle);
		void						DestroyProxy(Handle<VertexBuffer> handle);
		void						DestroyProxy(Handle<IndexBuffer> handle);
		void						DestroyProxy(Handle<UniformBuffer> handle);
		void						DestroyProxy(Handle<VertexArray> handle);
		void						DestroyProxy(Handle<CommandBuffer> handle);
		void						DestroyProxy(Handle<Framebuffer> handle);
		void						DestroyProxy(Handle<Shader> handle);

		// ------------------------------------------------------------------------------------- Draw ---------------------------------------------------------------------------------------

		// cache related
		Ref<VulkanRenderPass>		GetRenderPass(const RenderPassInfo renderPassInfo);
		Ref<VulkanPipeline>			GetRenderPipeline(const PipelineInfo spec, VkRenderPass renderPass);		


		void SetViewport(const glm::vec4 viewport) override;

		void BeginFrame() override;

		void EndFrame() override;

		void BindPipeline(Handle<Pipeline> pipeline) override;

		void BeginRenderPass(Handle<RenderPass> renderPass) override;

		void EndRenderPass() override;

		void Draw(Handle<Mesh> mesh, Handle<Material> material) override;

		void DrawIndexed(const Handle<VertexArray>& vertexArray, uint32_t count) override;

		operator VkDevice() {
			return m_Device;
		}

		// ------------------------------------------------------------------------------------ Uniform ----------------------------------------------------------

		VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetAllocateInfo& layout);

	private:

		void CheckRenderState();

		void SelectQueue(QueueFamilyIndices& indices);

		void CreatePresentQueue(const QueueFamilyIndices& indices);

	private:

		Ref<VulkanPhysicalDevice>		m_PhysicalDevice;
		VkDevice						m_Device;

		VkCommandPool					m_CommandPool;

		std::vector<VkDescriptorPool>	m_DescriptorPools;



		// can be used for transfer, graphics, compute
		// always use the first as the main queue for main render thread
		 		
		bool m_SupportAsyncCompute = false;
		bool m_SupportAsyncTransfer = false;


		VkQueue m_GraphicQueue;
		VkQueue m_ComputeQueue;
		VkQueue m_TransferQueue;
		VkQueue m_PresentQueue;

		DeviceResourceTable<Handle<Texture2D>,		Ref<VulkanTexture2D>>		m_Texture2DProxies;
		DeviceResourceTable<Handle<VertexBuffer>,	Ref<VulkanVertexBuffer>>	m_VertexBufferProxies;
		DeviceResourceTable<Handle<IndexBuffer>,	Ref<VulkanIndexBuffer>>		m_IndexBufferProxies;
		DeviceResourceTable<Handle<VertexArray>,	Ref<VulkanVertexArray>>		m_VertexArrayProxies;
		DeviceResourceTable<Handle<CommandBuffer>,	Ref<VulkanCommandBuffer>>	m_CommandBufferProxies;
		DeviceResourceTable<Handle<Framebuffer>,	Ref<VulkanFramebuffer>>		m_FramebufferProxies;
		DeviceResourceTable<Handle<Shader>,			Ref<VulkanShader>>			m_ShaderProxies;

		DeviceResourceTable<Handle<UniformBuffer>,	Ref<VulkanUniformBuffer>>	m_UniformBufferProxies;

		// todo change this
		struct RenderState {




			Ref<VulkanPipeline>		pipeline = nullptr;
			Ref<VulkanRenderPass>	renderPass = nullptr;
			Ref<VulkanFramebuffer>	renderTarget = nullptr;

			 
			// { offset.x, offset.y, width, height}
			glm::vec4 renderArea;
			glm::vec4 viewport;

 
		};	

		RenderState m_CurrentRenderState;
				
		// [frameIndex]
		std::vector<Scope<DescriptorSetManager>> m_DescriptorSetManagers;

		// every thread has its unique render resource
		VkDescriptorPool m_ImGuiDescriptorPool = nullptr;


		ByteKeyMap<Ref<VulkanRenderPass>>	m_RenderPassCache;
		ByteKeyMap<Ref<VulkanPipeline>>		m_RenderPipelineCache;		

		friend class VulkanSwapchain;
						
	};

}
