#include "hzpch.h"

#include "Platform/Vulkan/VulkanImGuiLayer.h"

#define IMGUI_IML_API


#include "Hazel/Renderer/RendererAPI.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_vulkan.h"



#include "Hazel/Core/Application.h"
#include "Platform/Windows/WindowsWindow.h"

#include "imgui.h"

#include "ImGuizmo.h"

namespace Hazel {



	void VulkanImGuiLayer::OnAttach()
	{
		//// Setup Dear ImGui context
		//IMGUI_CHECKVERSION();
		//ImGui::CreateContext();
		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		////io.ConfigViewportsNoAutoMerge = true;
		////io.ConfigViewportsNoTaskBarIcon = true;

		//// Setup Dear ImGui style
		//ImGui::StyleColorsDark();

		//// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		//ImGuiStyle& style = ImGui::GetStyle();
		//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//{
		//	style.WindowRounding = 0.0f;
		//	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		//}

		//GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNativeWindow());

		//// Setup Platform/Renderer backends
		//ImGui_ImplGlfw_InitForVulkan(window, true);
		//ImGui_ImplVulkan_InitInfo init_info = {};
		//init_info.Instance = g_Instance;
		//init_info.PhysicalDevice = g_PhysicalDevice;
		//init_info.Device = g_Device;
		//init_info.QueueFamily = g_QueueFamily;
		//init_info.Queue = g_Queue;
		//init_info.PipelineCache = g_PipelineCache;
		//init_info.DescriptorPool = g_DescriptorPool;
		//init_info.RenderPass = wd->RenderPass;
		//init_info.Subpass = 0;
		//init_info.MinImageCount = g_MinImageCount;
		//init_info.ImageCount = wd->ImageCount;
		//init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		//init_info.Allocator = g_Allocator;
		//init_info.CheckVkResultFn = check_vk_result;
		//ImGui_ImplVulkan_Init(&init_info);


	}

	void VulkanImGuiLayer::OnDetach()
	{
		// Cleanup
		VkResult result = vkDeviceWaitIdle(m_Device);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiLayer::Begin()
	{

	}

	void VulkanImGuiLayer::End()
	{
		//ImGui::Render();
		//ImDrawData* main_draw_data = ImGui::GetDrawData();
		//const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
		//wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
		//wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
		//wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
		//wd->ClearValue.color.float32[3] = clear_color.w;
		//if (!main_is_minimized)
		//	FrameRender(wd, main_draw_data);

		//// Update and Render additional Platform Windows
		//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//{
		//	ImGui::UpdatePlatformWindows();
		//	ImGui::RenderPlatformWindowsDefault();
		//}

		//// Present Main Platform Window
		//if (!main_is_minimized)
		//	FramePresent(wd);
	}

}
