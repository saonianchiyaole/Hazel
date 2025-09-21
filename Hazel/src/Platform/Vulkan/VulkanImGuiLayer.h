#pragma once

#include "Hazel/ImGui/ImGuiLayer.h"

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

	};

}
