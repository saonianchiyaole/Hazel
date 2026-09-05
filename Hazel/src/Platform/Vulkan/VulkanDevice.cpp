#include "hzpch.h"


#include "Hazel/Renderer/RenderPass.h"

#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

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
				indices.familys[queueFamilies[i].queueFlags] = { i, queueFamilies[i].queueCount };

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

				if (!indices.presentFamily.has_value() && presentSupport) {
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


	VulkanDevice::VulkanDevice()
	{

		Ref<VulkanPhysicalDevice> physicalDevice = MakeRef<VulkanPhysicalDevice>();

		Init(physicalDevice);

	}

	VulkanDevice::VulkanDevice(Ref<VulkanPhysicalDevice> physicalDevice) : m_PhysicalDevice(physicalDevice) {

		Init(physicalDevice);

	}

	bool VulkanDevice::Init(Ref<VulkanPhysicalDevice> physicalDevice) {

		QueueFamilyIndices& indices = physicalDevice->GetQueueFamilyIndices();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		// todo make it more flexible		

		SelectQueue(indices);


		float queuePriority = 1.0f;

		std::set<uint32_t> queueFamily = { indices.computeFamily.value(), indices.transferFamily.value(), indices.graphicsFamily.value() };

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

		VK_CHECK(vkCreateDevice(m_PhysicalDevice->GetRawPhysicalDevice(), &createInfo, nullptr, &m_Device));

		vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicQueue);
		vkGetDeviceQueue(m_Device, indices.transferFamily.value(), 0, &m_TransferQueue);
		vkGetDeviceQueue(m_Device, indices.computeFamily.value(), 0, &m_ComputeQueue);

		CreateCommandPool();
		
		// todo shouldn't be here
		Ref<VulkanSwapchain> swapchain = std::static_pointer_cast<VulkanSwapchain>(Application::GetInstance().GetWindow().GetSwapchain());
		//swapchain->CreateCommandBuffers();			

		// ------------------------------------  Create Descriptor Pool ------------------------------------  


		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10 * 3; // frames in flight should partially determine this
		poolInfo.poolSizeCount = (uint32_t)((int)(sizeof(poolSizes) / sizeof(*(poolSizes))));
		poolInfo.pPoolSizes = poolSizes;

		m_DescriptorPools.resize(m_FrameInFlight);
		for (uint32_t i = 0; i < m_FrameInFlight; i++) {
			HZ_CORE_ASSERT(
				vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPools[i]) == VK_SUCCESS,
				"Failed to create descriptor pool!"
			);
		}

		VK_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_ImGuiDescriptorPool));


		for (uint32_t i = 0; i < m_FrameInFlight; i++) {
			DescriptorSetManagerInfo descriptorSetManagerInfo{};
			descriptorSetManagerInfo.device = this;

			m_DescriptorSetManagers[i] = DescriptorSetManager::Create(descriptorSetManagerInfo);
		}		

		return true;
	}

	void VulkanDevice::Shutdown() {
		vkDeviceWaitIdle(m_Device);
	}

	void VulkanDevice::CreateCommandPool() {

		const QueueFamilyIndices& queueFamilyIndices = m_PhysicalDevice->GetQueueFamilyIndices();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool));

	}


	void VulkanDevice::CreateVertexBuffer(Handle<VertexBuffer> handle, size_t size, std::vector<uint8_t> data)
	{
		std::vector<uint8_t> proxyData = data;
		RenderDevice::CreateVertexBuffer(handle, size, std::move(data));
		CreateProxy(handle, size, std::move(proxyData));
	}

	void VulkanDevice::CreateIndexBuffer(Handle<IndexBuffer> handle, size_t size, std::vector<uint8_t> data)
	{
		std::vector<uint8_t> proxyData = data;
		RenderDevice::CreateIndexBuffer(handle, size, std::move(data));
		CreateProxy(handle, size, std::move(proxyData));
	}

	void VulkanDevice::CreateUniformBuffer(Handle<UniformBuffer> handle, const std::string name)
	{
		RenderDevice::CreateUniformBuffer(handle, name);
		CreateProxy(handle, name);
	}

	void VulkanDevice::CreateTexture2D(Handle<Texture2D> handle, TextureInfo textureInfo, std::vector<uint8_t> data)
	{
		std::vector<uint8_t> proxyData = std::move(data);
		CreateProxy(handle, textureInfo, std::move(proxyData));
	}

	Ref<VulkanCommandBuffer> VulkanDevice::CreateCommandBufferRef() {

		Ref<VulkanCommandBuffer> commandBuffer = MakeRef<VulkanCommandBuffer>(m_CommandPool);
		return commandBuffer;
	}

	Ref<VulkanCommandBuffer> VulkanDevice::CreateSecondaryCommandBufferRef() {

		Ref<VulkanCommandBuffer> commandBuffer = MakeRef<VulkanCommandBuffer>(m_CommandPool, false);
		return commandBuffer;
	}

	void VulkanDevice::RegisterProxy(Handle<Texture2D> handle, Ref<VulkanTexture2D> proxy)
	{
		m_Texture2DProxies.Add(handle, proxy);
	}

	Ref<VulkanTexture2D> VulkanDevice::GetProxy(Handle<Texture2D> handle)
	{
		Ref<VulkanTexture2D>* proxy = m_Texture2DProxies.Get(handle);
		return proxy ? *proxy : nullptr;
	}

	bool VulkanDevice::HasProxy(Handle<Texture2D> handle) const
	{
		return m_Texture2DProxies.Has(handle);
	}

	void VulkanDevice::DestroyProxy(Handle<Texture2D> handle)
	{
		m_Texture2DProxies.Remove(handle);
	}

	void VulkanDevice::RegisterProxy(Handle<VertexBuffer> handle, Ref<VulkanVertexBuffer> proxy)
	{
		m_VertexBufferProxies.Add(handle, proxy);
	}

	Ref<VulkanVertexBuffer> VulkanDevice::GetProxy(Handle<VertexBuffer> handle)
	{
		Ref<VulkanVertexBuffer>* proxy = m_VertexBufferProxies.Get(handle);
		return proxy ? *proxy : nullptr;
	}

	bool VulkanDevice::HasProxy(Handle<VertexBuffer> handle) const
	{
		return m_VertexBufferProxies.Has(handle);
	}

	void VulkanDevice::DestroyProxy(Handle<VertexBuffer> handle)
	{
		m_VertexBufferProxies.Remove(handle);
	}

	void VulkanDevice::RegisterProxy(Handle<IndexBuffer> handle, Ref<VulkanIndexBuffer> proxy)
	{
		m_IndexBufferProxies.Add(handle, proxy);
	}

	Ref<VulkanIndexBuffer> VulkanDevice::GetProxy(Handle<IndexBuffer> handle)
	{
		Ref<VulkanIndexBuffer>* proxy = m_IndexBufferProxies.Get(handle);
		return proxy ? *proxy : nullptr;
	}

	bool VulkanDevice::HasProxy(Handle<IndexBuffer> handle) const
	{
		return m_IndexBufferProxies.Has(handle);
	}

	void VulkanDevice::DestroyProxy(Handle<IndexBuffer> handle)
	{
		m_IndexBufferProxies.Remove(handle);
	}

	void VulkanDevice::RegisterProxy(Handle<UniformBuffer> handle, Ref<VulkanUniformBuffer> proxy)
	{
		m_UniformBufferProxies.Add(handle, proxy);
	}

	Ref<VulkanUniformBuffer> VulkanDevice::GetProxy(Handle<UniformBuffer> handle)
	{
		Ref<VulkanUniformBuffer>* proxy = m_UniformBufferProxies.Get(handle);
		return proxy ? *proxy : nullptr;
	}

	bool VulkanDevice::HasProxy(Handle<UniformBuffer> handle) const
	{
		return m_UniformBufferProxies.Has(handle);
	}

	void VulkanDevice::DestroyProxy(Handle<UniformBuffer> handle)
	{
		m_UniformBufferProxies.Remove(handle);
	}

	void VulkanDevice::RegisterProxy(Handle<VertexArray> handle, Ref<VulkanVertexArray> proxy)
	{
		m_VertexArrayProxies.Add(handle, proxy);
	}

	Ref<VulkanVertexArray> VulkanDevice::GetProxy(Handle<VertexArray> handle)
	{
		Ref<VulkanVertexArray>* proxy = m_VertexArrayProxies.Get(handle);
		return proxy ? *proxy : nullptr;
	}

	bool VulkanDevice::HasProxy(Handle<VertexArray> handle) const
	{
		return m_VertexArrayProxies.Has(handle);
	}

	void VulkanDevice::DestroyProxy(Handle<VertexArray> handle)
	{
		m_VertexArrayProxies.Remove(handle);
	}

	void VulkanDevice::RegisterProxy(Handle<CommandBuffer> handle, Ref<VulkanCommandBuffer> proxy)
	{
		m_CommandBufferProxies.Add(handle, proxy);
	}

	Ref<VulkanCommandBuffer> VulkanDevice::GetProxy(Handle<CommandBuffer> handle)
	{
		Ref<VulkanCommandBuffer>* proxy = m_CommandBufferProxies.Get(handle);
		return proxy ? *proxy : nullptr;
	}

	bool VulkanDevice::HasProxy(Handle<CommandBuffer> handle) const
	{
		return m_CommandBufferProxies.Has(handle);
	}

	void VulkanDevice::DestroyProxy(Handle<CommandBuffer> handle)
	{
		m_CommandBufferProxies.Remove(handle);
	}

	void VulkanDevice::CreateShader(Handle<Shader> handle, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		RenderDevice::CreateShader(handle, vertexSrc, fragmentSrc);
		CreateProxy(handle, GetResource(handle)->CreateSnapShot());
	}

	void VulkanDevice::CreateShader(Handle<Shader> handle, const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		RenderDevice::CreateShader(handle, name, vertexSrc, fragmentSrc);
		CreateProxy(handle, GetResource(handle)->CreateSnapShot());
	}

	void VulkanDevice::CreateShader(Handle<Shader> handle, const std::string& filepath)
	{
		RenderDevice::CreateShader(handle, filepath);
		CreateProxy(handle, GetResource(handle)->CreateSnapShot());
	}

	void VulkanDevice::CreateProxy(Handle<Texture2D> handle, TextureInfo textureInfo, std::vector<uint8_t> data)
	{
		RegisterProxy(handle, MakeRef<VulkanTexture2D>(textureInfo, std::move(data)));
	}

	void VulkanDevice::CreateProxy(Handle<VertexBuffer> handle, size_t size, std::vector<uint8_t> data)
	{
		RegisterProxy(handle, MakeRef<VulkanVertexBuffer>(size, std::move(data)));
	}

	void VulkanDevice::CreateProxy(Handle<IndexBuffer> handle, size_t size, std::vector<uint8_t> data)
	{
		RegisterProxy(handle, MakeRef<VulkanIndexBuffer>(size, std::move(data)));
	}

	void VulkanDevice::CreateProxy(Handle<UniformBuffer> handle, size_t size, std::vector<uint8_t> data)
	{
		RegisterProxy(handle, MakeRef<VulkanUniformBuffer>(size, data));
	}



	void VulkanDevice::CreateProxy(Handle<VertexArray> handle)
	{
		RegisterProxy(handle, MakeRef<VulkanVertexArray>());
	}

	void VulkanDevice::CreateProxy(Handle<CommandBuffer> handle, bool primary)
	{
		RegisterProxy(handle, primary ? MakeRef<VulkanCommandBuffer>(m_CommandPool) : MakeRef<VulkanCommandBuffer>(m_CommandPool, false));
	}

	void VulkanDevice::CreateProxy(Handle<Framebuffer> handle, const FramebufferInfo info)
	{
		RenderPassInfo renderPassInfo;
		renderPassInfo.attachmentInfos = info.attachments;
		Ref<VulkanRenderPass> renderPass = GetRenderPass(renderPassInfo);

		RegisterProxy(handle, MakeRef<VulkanFramebuffer>(info));
	}

	void VulkanDevice::CreateProxy(Handle<Shader> handle, Ref<const ShaderSnapshot> shaderSnapshot)
	{
		RegisterProxy(handle, MakeRef<VulkanShader>(shaderSnapshot));
	}


	void VulkanDevice::RegisterProxy(Handle<Framebuffer> handle, Ref<VulkanFramebuffer> proxy)
	{
		m_FramebufferProxies.Add(handle, proxy);
	}

	Ref<VulkanFramebuffer> VulkanDevice::GetProxy(Handle<Framebuffer> handle)
	{
		Ref<VulkanFramebuffer>* proxy = m_FramebufferProxies.Get(handle);
		return proxy ? *proxy : nullptr;
	}

	bool VulkanDevice::HasProxy(Handle<Framebuffer> handle) const
	{
		return m_FramebufferProxies.Has(handle);
	}

	void VulkanDevice::DestroyProxy(Handle<Framebuffer> handle)
	{
		m_FramebufferProxies.Remove(handle);
	}

	void VulkanDevice::RegisterProxy(Handle<Shader> handle, Ref<VulkanShader> proxy)
	{
		m_ShaderProxies.Add(handle, proxy);
	}

	Ref<VulkanShader> VulkanDevice::GetProxy(Handle<Shader> handle)
	{
		Ref<VulkanShader>* proxy = m_ShaderProxies.Get(handle);
		return proxy ? *proxy : nullptr;
	}

	bool VulkanDevice::HasProxy(Handle<Shader> handle) const
	{
		return m_ShaderProxies.Has(handle);
	}

	void VulkanDevice::DestroyProxy(Handle<Shader> handle)
	{
		m_ShaderProxies.Remove(handle);
	}


	// ------------------------------------------------------------------------------------- Draw ---------------------------------------------------------------------------------------






	// ------------------------------------------------------------------------------------- VulkanResource ---------------------------------------------------------------------------------------

	// cache related
	Ref<VulkanRenderPass> VulkanDevice::GetRenderPass(const RenderPassInfo info) {
		ByteKey key = std::move(Utils::GetRenderPassByteKey(info));

		if (m_RenderPassCache.find(key) != m_RenderPassCache.end()) {
			return m_RenderPassCache[key];
		}

		m_RenderPassCache[key] = MakeRef<VulkanRenderPass>(info);

		return m_RenderPassCache[key];
	}


	Ref<VulkanPipeline> VulkanDevice::GetRenderPipeline(const PipelineInfo info, VkRenderPass renderPass) {

		ByteKey key = Utils::GetRenderPipelineByteKey(info, renderPass);

		if (m_RenderPipelineCache.find(key) != m_RenderPipelineCache.end()) {
			return m_RenderPipelineCache[key];
		}

		m_RenderPipelineCache[key] = VulkanPipeline::CreateVulkanPipeline(info, renderPass);

		return m_RenderPipelineCache[key];
	}

	void VulkanDevice::SetViewport(const glm::vec4 viewport)
	{
		m_CurrentRenderState.viewport = viewport;
	}

	void VulkanDevice::BeginFrame()
	{
		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();
		vkResetDescriptorPool(m_Device, m_DescriptorPools[frameIndex], 0);
	}

	void VulkanDevice::EndFrame()
	{

	}

	void VulkanDevice::BindPipeline(Handle<Pipeline> pipeline)
	{
		if (!HasResource(pipeline)) {
			return;
		}

		PipelineInfo pipelineSpec = GetResource(pipeline)->GetSpecification();

		auto rendGetProxy();

		if (m_CurrentRenderState.renderPass) {

			Ref<VulkanPipeline> vulkanPipeline = GetRenderPipeline(pipelineSpec, m_CurrentRenderState.renderPass->GetRawRenderPass());

			if (m_CurrentRenderState.pipeline == vulkanPipeline) {
				return;
			}
			else {

			}

		}

	}


	void VulkanDevice::BeginRenderPass(Handle<RenderPass> renderPass)
	{
		if (!HasResource(renderPass)) {
			return;
		}

		RenderPassInfo info = GetResource(renderPass)->GetInfo();
		Ref<VulkanRenderPass> vulkanRenderPass = GetRenderPass(info);

		m_CurrentRenderState.renderPass = vulkanRenderPass;

	}

	void VulkanDevice::EndRenderPass()
	{
		bool valid = true;

		if (!m_CurrentRenderState.pipeline) {
			HZ_CORE_WARN("Set Pipeline before render!");
			valid = false;
		}
		if (!m_CurrentRenderState.renderPass) {
			HZ_CORE_WARN("Start renderPass before render!");
			valid = false;
		}

		if (!m_CurrentRenderState.renderTarget) {
			HZ_CORE_WARN("Render Target is NULL!");
			valid = false;
		}

		if (!valid) {
			return;
		}
		
		Ref<VulkanCommandBuffer> vulkanCommandBuffer = MakeRef<VulkanCommandBuffer>();
		Ref<VulkanRenderPass>	renderPass = m_CurrentRenderState.renderPass;
		Ref<VulkanPipeline>		pipeline = m_CurrentRenderState.pipeline;
		Ref<VulkanFramebuffer>	renderTarget = m_CurrentRenderState.renderTarget;
		glm::vec4& renderArea = m_CurrentRenderState.renderArea;
		glm::vec4& viewport = m_CurrentRenderState.viewport;

		const PipelineInfo& pipelineInfo = pipeline->GetPipelineInfo();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass->GetRawRenderPass();
		renderPassInfo.framebuffer = renderTarget->GetRawFramebuffer();

		renderPassInfo.renderArea.offset = { (int32_t)renderArea.x, (int32_t)renderArea.y };
		renderPassInfo.renderArea.extent = VkExtent2D{ (uint32_t)renderArea.z, (uint32_t)renderArea.w };


		std::array<VkClearValue, 2> clearColor;
		clearColor[0].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
		clearColor[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = clearColor.size();
		renderPassInfo.pClearValues = clearColor.data();

		vulkanCommandBuffer->Begin();

		vkCmdBeginRenderPass(vulkanCommandBuffer->GetRawCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkPipeline rawPipeline = pipeline->GetRawPipeline();

		vkCmdBindPipeline(vulkanCommandBuffer->GetRawCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, rawPipeline);

		std::vector<VkDescriptorSet> descriptorSets = std::move(m_DescriptorSetManagers[m_FrameIndex]->GetSortedDescriptorSets());

		vkCmdBindDescriptorSets(vulkanCommandBuffer->GetRawCommandBuffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->GetPipelineLayout(),
			0,
			descriptorSets.size(),
			descriptorSets.data(),
			0,  // for now we don't use dynamic offsets and we make sure that all UBOs are continuous by set
			nullptr);


		// dynamic part
		VkViewport vkViewport{};
		vkViewport.x = viewport.x;
		vkViewport.y = viewport.y;
		vkViewport.width = viewport.z;
		vkViewport.height = viewport.w;
		vkViewport.minDepth = 0.0f;
		vkViewport.maxDepth = 1.0f;
		vkCmdSetViewport(vulkanCommandBuffer->GetRawCommandBuffer(), 0, 1, &vkViewport);

		VkRect2D scissor{};
		scissor.offset = { (int32_t)renderArea.x, (int32_t)renderArea.y };
		scissor.extent = { (uint32_t)renderArea.w, (uint32_t)renderArea.z };
		vkCmdSetScissor(vulkanCommandBuffer->GetRawCommandBuffer(), 0, 1, &scissor);

		vkCmdSetLineWidth(vulkanCommandBuffer->GetRawCommandBuffer(), pipeline->GetPipelineInfo().lineWidth);


		vulkanCommandBuffer->End();

		m_CurrentRenderState.renderPass = nullptr;
	}

	void VulkanDevice::Draw(Handle<Mesh> mesh, Handle<Material> material)
	{
	}

	void VulkanDevice::DrawIndexed(const Handle<VertexArray>& vertexArray, uint32_t count)
	{
	}


	// ------------------------------------------------------------------------------------- Device ---------------------------------------------------------------------------------------

	VkDescriptorSet VulkanDevice::AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{		
		VkDescriptorSet descriptorSet;
		allocInfo.descriptorPool = m_DescriptorPools[m_FrameIndex];
		vkAllocateDescriptorSets(m_Device, &allocInfo, &descriptorSet);
		return descriptorSet;
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

	void VulkanDevice::CreatePresentQueue(const QueueFamilyIndices& indices) {

		vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);

	}

}
