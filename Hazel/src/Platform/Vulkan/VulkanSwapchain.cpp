#include "hzpch.h"

#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"


#include "Hazel/Renderer/RenderPass.h"
#include "Hazel/Renderer/Framebuffer.h"


#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan_win32.h>

namespace Hazel {


	namespace Utils {


		SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
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



		SwapchainDetails ChooseSwapchain(const SwapchainSupportDetails swapChainSupportDetails, GLFWwindow* window) {
			SwapchainDetails swapChainDetail;
			swapChainDetail.presentModle = ChooseSwapPresentMode(swapChainSupportDetails.presentModes);
			swapChainDetail.surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupportDetails.formats);
			swapChainDetail.swapChainExtent = ChooseSwapExtent(swapChainSupportDetails.capabilities, window);
			return swapChainDetail;
		}

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

			for (const auto& availableFormat : availableFormats) {

				if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM) {
					return availableFormat;
				}
			}
			return availableFormats[0];

		}

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {

			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
				return capabilities.currentExtent;
			}
			else {
				int width, height;
				glfwGetFramebufferSize(window, &width, &height);

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}

		}

	}


	void VulkanSwapchain::Destroy()
	{
		uint32_t frameInFlight = Renderer::GetFrameInFlight();

		VkDevice device = m_Device->GetRawDevice();

		vkDeviceWaitIdle(device);

		for (auto framebuffer : m_Framebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		for (auto imageView : m_ImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}			

		for (uint32_t i = 0; i < frameInFlight; i++) {
			vkDestroyFence(device, m_InFlightFences[i], nullptr);
			vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
		}

	
		vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
		m_Swapchain = VK_NULL_HANDLE;
		
	}

	

	void VulkanSwapchain::WaitFrameFence()
	{
		vkWaitForFences(m_Device->GetRawDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);
	}

	void VulkanSwapchain::WaitAndResetFrameFence()
	{
		vkWaitForFences(m_Device->GetRawDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);
		vkResetFences(m_Device->GetRawDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex]);
	}

	void VulkanSwapchain::BeginFrame()
	{

		AcquireNextImage();		
				

	}

	void VulkanSwapchain::EndFrame()
	{

	}

	

	void VulkanSwapchain::Create(uint32_t width, uint32_t height, bool isVsync) {


		SwapchainSupportDetails swapChainSupport = Utils::QuerySwapchainSupport(m_Device->GetPhysicalDevice()->GetRawPhysicalDevice(), m_Surface);

		m_Details = Utils::ChooseSwapchain(swapChainSupport, m_Window);

		VkExtent2D extent = {
			width,
			height
		};

		m_Details.swapChainExtent = extent;

		VkSurfaceFormatKHR surfaceFormat = m_Details.surfaceFormat;
		VkPresentModeKHR presentMode = m_Details.presentModle;
		VkExtent2D swapExtent = m_Details.swapChainExtent;

		m_ImageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && m_ImageCount > swapChainSupport.capabilities.maxImageCount) {
			m_ImageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = m_ImageCount;
		createInfo.imageFormat = m_Details.surfaceFormat.format;
		createInfo.imageColorSpace = m_Details.surfaceFormat.colorSpace;
		createInfo.imageExtent = swapExtent;


		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		std::vector<uint32_t> queueFamilyIndices(m_VisibleQueueFamily.begin(), m_VisibleQueueFamily.end());		

		if (queueFamilyIndices.size() == 1) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else {

			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = m_VisibleQueueFamily.size();
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		
		VkSwapchainKHR oldSwapchain = m_Swapchain;		

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;		
		
		VkResult result = vkCreateSwapchainKHR(m_Device->GetRawDevice(), &createInfo, nullptr, &m_Swapchain);
		if (result != VK_SUCCESS)
			throw std::runtime_error("failed to create swap chain!");

		
		if(oldSwapchain)
			vkDestroySwapchainKHR(m_Device->GetRawDevice(), oldSwapchain, nullptr);
		
		m_Images.clear();
		vkGetSwapchainImagesKHR(m_Device->GetRawDevice(), m_Swapchain, &m_ImageCount, nullptr);
		m_Images.resize(m_ImageCount);
		vkGetSwapchainImagesKHR(m_Device->GetRawDevice(), m_Swapchain, &m_ImageCount, m_Images.data());

		CreateImageViews();
		CreateFramebuffers();
		CreateSyncObjects();		
	}


	void VulkanSwapchain::Recreate(uint32_t width, uint32_t height, bool isVsync) {

		vkDeviceWaitIdle(m_Device->GetRawDevice());

		Destroy();

		Create(width, height, isVsync);		

		vkDeviceWaitIdle(m_Device->GetRawDevice());		

		m_IsRebuilt = true;
				
	}

	void VulkanSwapchain::InitializeSurface(VkInstance instance, GLFWwindow* window) {

		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = glfwGetWin32Window(window); //´°¿Ú¾ä±ú
		createInfo.hinstance = GetModuleHandle(nullptr); //½ø³Ì¾ä±ú


		VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &m_Surface);

		if (result != VK_SUCCESS)
			HZ_CORE_ASSERT(false, "Failed to create window surface");


	}

	void VulkanSwapchain::Init(Ref<VulkanContext> context)
	{
		m_Device = context->GetDevice();


		Ref<VulkanPhysicalDevice> physicalDevice = context->GetPhysicalDevice();
		QueueFamilyIndices& indices = physicalDevice->GetQueueFamilyIndices();		
		
		Utils::PickPresentFamilyIndex(indices, physicalDevice->GetRawPhysicalDevice(), m_Surface);

		m_Device->CreatePresentQueue(indices);

		m_VisibleQueueFamily.clear();
		m_VisibleQueueFamily.insert(indices.graphicsFamily.value());
		m_VisibleQueueFamily.insert(indices.presentFamily.value());
	}

	void VulkanSwapchain::CreateImageViews() {

		m_ImageViews.resize(m_ImageCount);

		for (int i = 0; i < m_ImageCount; i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_Images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_Details.surfaceFormat.format;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;


			if (vkCreateImageView(m_Device->GetRawDevice(), &createInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}

		}

	}


	/*void VulkanSwapchain::CreateRenderPass() {

		RenderPassSpec spec = { nullptr };



		RenderPass::Create(spec);



	}*/

	void VulkanSwapchain::CreateSyncObjects() {
		
		uint32_t frameInFilght = Renderer::GetFrameInFlight();
		m_RenderFinishedSemaphores.resize(frameInFilght);
		m_ImageAvailableSemaphores.resize(frameInFilght);
		m_InFlightFences.resize(frameInFilght);


		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < frameInFilght; i++) {
			if (vkCreateSemaphore(m_Device->GetRawDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS
				|| vkCreateSemaphore(m_Device->GetRawDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS
				|| vkCreateFence(m_Device->GetRawDevice(), &fenceInfo, nullptr, &m_InFlightFences[i])) {

				throw std::runtime_error("failed to create semaphores!");
			}
		}

	}

	void VulkanSwapchain::CreateCommandBuffers()
	{

		uint32_t frameInFilght = Renderer::GetFrameInFlight();

		m_CommandBuffers.resize(frameInFilght);
		for (int i = 0; i < frameInFilght; i++) {
			m_CommandBuffers[i] = MakeRef<VulkanCommandBuffer>();
		}

	}


	void VulkanSwapchain::CreateFramebuffers() {


		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_Details.surfaceFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;	


		if (vkCreateRenderPass(m_Device->GetRawDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
			HZ_CORE_ASSERT(false, "Fail to create render pass!");
		}

		m_Framebuffers.resize(m_ImageCount);

		for (int i = 0; i < m_ImageCount; i++) {

			VkImageView attachments[] = {
				m_ImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_Details.swapChainExtent.width;
			framebufferInfo.height = m_Details.swapChainExtent.height;
			framebufferInfo.layers = 1;			

			if (vkCreateFramebuffer(m_Device->GetRawDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}

		}


	}

	uint32_t VulkanSwapchain::AcquireNextImage()
	{

		Renderer::SubmitTask([this]() {


			m_CurrentFrameIndex;

			vkWaitForFences(m_Device->GetRawDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);
			

			VkResult result = vkAcquireNextImageKHR(m_Device->GetRawDevice(), m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrameIndex], VK_NULL_HANDLE, &m_CurrentImageIndex);

			if (!Application::GetInstance().GetWindow().IsMinimized() && (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)) {

				Recreate(Application::GetInstance().GetWindow().GetWidth(), Application::GetInstance().GetWindow().GetHeight(), Application::GetInstance().GetWindow().IsVSync());

				result = vkAcquireNextImageKHR(m_Device->GetRawDevice(), m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrameIndex], VK_NULL_HANDLE, &m_CurrentImageIndex);
			}
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && !Application::GetInstance().GetWindow().IsMinimized()) {

				throw std::runtime_error("failed to acquire swap chain image");
			}

			vkResetFences(m_Device->GetRawDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex]);

			m_IsRebuilt = false;

			vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrameIndex]->GetRawCommandBuffer(), 0);


			});

		return m_CurrentImageIndex;

	}

	void VulkanSwapchain::Present() {


		Renderer::SubmitTask([this]() {

			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[m_CurrentFrameIndex];
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrameIndex]->GetRawCommandBuffer();
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrameIndex];

			if (vkQueueSubmit(m_Device->GetGraphicQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrameIndex]) != VK_SUCCESS) {
				throw std::runtime_error("failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrameIndex];
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &m_Swapchain;
			presentInfo.pImageIndices = &m_CurrentImageIndex;

			//presentInfo.pResults = nullptr;
			VkResult result = vkQueuePresentKHR(m_Device->GetPresentQueue(), &presentInfo);

			if ((result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) && !Application::GetInstance().GetWindow().IsMinimized()) {
				Recreate(Application::GetInstance().GetWindow().GetWidth(), Application::GetInstance().GetWindow().GetHeight(), Application::GetInstance().GetWindow().IsVSync());

			}
			else if (result != VK_SUCCESS && !Application::GetInstance().GetWindow().IsMinimized()) {
				throw std::runtime_error("failed to present swap chain image!");
			}

			m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % Renderer::GetFrameInFlight();

			Application::GetInstance().NextRenderFrame();

			m_IsRebuilt = false;

			});



	}


}


