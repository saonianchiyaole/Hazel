#pragma once

#include "Hazel/ImGui/ImGuiLayer.h"

#include <vulkan/vulkan.h>
#include "backends/imgui_impl_vulkan.h"

namespace Hazel {

		

	class VulkanImGuiLayer: public ImGuiLayer {

	public:
		VulkanImGuiLayer() = default;
		~VulkanImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender();

		virtual void Begin() override;
		virtual void End() override;

	private:

		VkDevice m_Device;		
		ImGui_ImplVulkanH_Window* wd;

	};

}
