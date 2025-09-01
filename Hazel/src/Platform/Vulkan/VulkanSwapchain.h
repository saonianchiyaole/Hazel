#pragma once

#include "vulkan/vulkan.h"
#include "Hazel/Renderer/Swapchain.h"

struct GLFWwindow;

namespace Hazel {
	
	class VulkanDevice;
	class VulkanFramebuffer;
	class VulkanRenderPass;
	class VulkanCommandBuffer;
	class VulkanContext;

	struct SwapchainDetails {
		VkSurfaceFormatKHR	surfaceFormat;
		VkPresentModeKHR	presentModle;
		VkExtent2D			swapChainExtent;
	};

	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR			capabilities;
		std::vector<VkSurfaceFormatKHR>		formats;
		std::vector<VkPresentModeKHR>		presentModes;
	};


	namespace Utils {

		SwapchainSupportDetails		QuerySwapchainSupport	(VkPhysicalDevice device, VkSurfaceKHR surface);
		SwapchainDetails			ChooseSwapchain			(const SwapchainSupportDetails swapChainSupportDetails, GLFWwindow* window);
		VkSurfaceFormatKHR			ChooseSwapSurfaceFormat	(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR			ChooseSwapPresentMode	(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D					ChooseSwapExtent		(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	}


	class VulkanSwapchain : public Swapchain{


	public:

		VulkanSwapchain() = default;

		VkSwapchainKHR				GetRawSwapchain()					{ return m_Swapchain; }
		SwapchainDetails			GetDetails()						{ return m_Details; }
		std::vector<VkFramebuffer>	GetFramebuffers()					{ return m_Framebuffers; }

		VkRenderPass				GetRenderPass()						{ return m_RenderPass; }
		VkSurfaceKHR				GetSurface()						{ return m_Surface; }



		std::vector<VkSemaphore>&	GetImageAvailableSemaphores()		{ return m_ImageAvailableSemaphores; }
		std::vector<VkSemaphore>&	GetRenderFinishedSemaphores()		{ return m_RenderFinishedSemaphores; }
		std::vector<VkFence>&		GetInFlightFences()					{ return m_InFlightFences; }

		Ref<VulkanCommandBuffer>	GetCurrentCommandBuffer()			{ return m_CommandBuffers[m_CurrentFrameIndex]; }		
		VkFramebuffer				GetCurrentFramebuffer()				{ return m_Framebuffers[m_CurrentFrameIndex]; }

		void						InitializeSurface		(VkInstance instance, GLFWwindow* window);
		void						Init					(Ref<VulkanDevice> device);
		void						Create					(uint32_t width, uint32_t height, bool isVsync);
		void						Recreate				(uint32_t width, uint32_t height, bool isVsync);
		void						Destroy					();
		

		virtual void BeginFrame()	override;
		virtual void EndFrame()		override;
		virtual void Present()		override;

	private:

		uint32_t	AcquireNextImage() override;
		void		CreateImageViews();
		void		CreateFramebuffers();		
		void		CreateSyncObjects();
		void		CreateCommandBuffers();

	private:

		SwapchainDetails m_Details;

		GLFWwindow* m_Window;
		VkInstance m_VulkanInstance;

		Ref<VulkanDevice> m_Device = nullptr;

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;

		// this renderpass is used to render the final pixel to framebuffer
		VkRenderPass m_RenderPass = nullptr;

		VkSwapchainKHR m_Swapchain = nullptr;
		VkSurfaceKHR m_Surface = nullptr;
		
		// Semaphore

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;

		// used to render a final frame 
		std::vector<Ref<VulkanCommandBuffer>> m_CommandBuffers;

		//Fence

		std::vector<VkFence> m_InFlightFences;

		friend class VulkanRendererAPI;
	};


}
