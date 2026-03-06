#pragma once

#include "Hazel/ImGui/ImGuiLayer.h"

#include "imgui_threaded_rendering.h"

#include <chrono>

namespace Hazel {

	class VulkanCommandBuffer;

	class VulkanImGuiLayer: public ImGuiLayer {

	public:
		VulkanImGuiLayer() = default;
		~VulkanImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		virtual void Begin() override;
		virtual void End() override;

	private:

		std::vector<Ref<VulkanCommandBuffer>> m_CommandBuffers;				

		std::queue<ImGui::ImDrawDataSnapshot> m_SnapShots;
		
		std::chrono::steady_clock::time_point m_StartTime;

		std::mutex m_SnapshotMutex;

	};

}
