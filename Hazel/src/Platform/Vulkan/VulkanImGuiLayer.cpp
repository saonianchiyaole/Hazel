#include "hzpch.h"

#include "Platform/Vulkan/VulkanImGuiLayer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"


#ifndef IMGUI_IML_API
#define IMGUI_IML_API
#endif

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Hazel/Core/Application.h"
#include "Platform/Windows/WindowsWindow.h"

#include "ImGuizmo.h"

namespace Hazel {


	void VulkanImGuiLayer::OnAttach()
	{
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			m_CommandBuffers[i] = VulkanContext::GetCurrentContext()->GetDevice()->CreateSecondaryCommandBuffer();
		}
		
		//// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		io.ConfigViewportsNoAutoMerge = true;
		io.ConfigViewportsNoTaskBarIcon = true;

		//// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		//// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());

		Ref<VulkanContext> context = VulkanContext::GetCurrentContext();
		VkInstance instance = context->GetVulkanInstance();
		VkPhysicalDevice  physicalDevice = context->GetPhysicalDevice()->GetRawPhysicalDevice();
		VkDevice device = context->GetDevice()->GetRawDevice();
		VkQueue graphicQueue = context->GetDevice()->GetGraphicQueue();
		QueueFamilyIndices queueFamily = context->GetPhysicalDevice()->GetQueueFamilyIndices();
		Ref<VulkanSwapchain> swapchain = std::dynamic_pointer_cast<VulkanSwapchain>(Application::GetInstance().GetWindow().GetSwapchain());


		// Create Descriptor Pool
		VkDescriptorPool descriptorPool;
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
		poolInfo.maxSets = 1000 * 3; // frames in flight should partially determine this
		poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		HZ_CORE_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");
		

		//// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = instance;
		init_info.PhysicalDevice = physicalDevice;
		init_info.Device = device;
		init_info.QueueFamily = queueFamily.graphicsFamily.value();
		init_info.Queue = graphicQueue;
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = descriptorPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;		
		init_info.ImageCount = swapchain->GetImageCount();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = nullptr;
		init_info.RenderPass = swapchain->GetRenderPass();		
		ImGui_ImplVulkan_Init(&init_info);
		
		ImGui_ImplVulkan_CreateFontsTexture();

	}

	void VulkanImGuiLayer::OnDetach()
	{		
		// Cleanup
		VkResult result = vkDeviceWaitIdle(VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice());
		
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();
	}

	void VulkanImGuiLayer::End()
	{
		ImGui::Render();
		
		VkClearValue clearValues[1];
		static Ref<VulkanSwapchain> swapchain = std::static_pointer_cast<VulkanSwapchain>(Application::GetInstance().GetWindow().GetSwapchain());
		Ref<VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		


		clearValues[0].color = { {1.0f, 0.1f, 0.1f, 1.0f} };
		//clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapchain->GetDetails().swapChainExtent.width;
		uint32_t height = swapchain->GetDetails().swapChainExtent.height;

		const uint32_t frameIndex = swapchain->GetCurrentFrameIndex();

		
		

		Ref<VulkanCommandBuffer> cmd = swapchain->GetCurrentCommandBuffer();
		VkCommandBuffer drawCommandBuffer = swapchain->GetCurrentCommandBuffer()->GetRawCommandBuffer();
		VkCommandBuffer imGuiCommandBuffer = m_CommandBuffers[frameIndex]->GetRawCommandBuffer();
		
		cmd->Begin();


		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = swapchain->GetRenderPass();
		info.framebuffer = swapchain->GetCurrentFramebuffer();
		info.renderArea.extent.width = width;
		info.renderArea.extent.height = height;
		info.clearValueCount = 1;
		info.pClearValues = clearValues;
		vkCmdBeginRenderPass(drawCommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

		ImDrawData* main_draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(main_draw_data, drawCommandBuffer);		


		// Submit command buffer
		vkCmdEndRenderPass(drawCommandBuffer);
		

		cmd->End();

		//VkRenderPassBeginInfo renderPassBeginInfo = {};
		//renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//renderPassBeginInfo.pNext = nullptr;
		//renderPassBeginInfo.renderPass = swapchain->GetRenderPass();
		//renderPassBeginInfo.renderArea.offset.x = 0;
		//renderPassBeginInfo.renderArea.offset.y = 0;
		//renderPassBeginInfo.renderArea.extent.width = width;
		//renderPassBeginInfo.renderArea.extent.height = height;
		//renderPassBeginInfo.clearValueCount = 2; // Color + depth
		//renderPassBeginInfo.pClearValues = clearValues;
		//renderPassBeginInfo.framebuffer = swapchain->GetCurrentFramebuffer();

		//vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		//VkCommandBufferInheritanceInfo inheritanceInfo = {};
		//inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		//inheritanceInfo.renderPass = swapchain->GetRenderPass();
		//inheritanceInfo.framebuffer = swapchain->GetCurrentFramebuffer();

		//VkCommandBufferBeginInfo cmdBufInfo = {};
		//cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		//cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

		//vkBeginCommandBuffer(imGuiCommandBuffer, &cmdBufInfo);

		//VkViewport viewport = {};
		//viewport.x = 0.0f;
		//viewport.y = (float)height;
		//viewport.height = -(float)height;
		//viewport.width = (float)width;
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;
		//vkCmdSetViewport(imGuiCommandBuffer, 0, 1, &viewport);

		//VkRect2D scissor = {};
		//scissor.extent.width = width;
		//scissor.extent.height = height;
		//scissor.offset.x = 0;
		//scissor.offset.y = 0;
		//vkCmdSetScissor(imGuiCommandBuffer, 0, 1, &scissor);


		//ImDrawData* main_draw_data = ImGui::GetDrawData();
		//ImGui_ImplVulkan_RenderDrawData(main_draw_data, imGuiCommandBuffer);

		//vkEndCommandBuffer(imGuiCommandBuffer);

		//std::vector<VkCommandBuffer> commandBuffers;
		//commandBuffers.push_back(imGuiCommandBuffer);

		//vkCmdExecuteCommands(drawCommandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());

		//vkCmdEndRenderPass(drawCommandBuffer);

		//vkEndCommandBuffer(drawCommandBuffer);
						
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanImGuiLayer::OnImGuiRender() {

	}



}
