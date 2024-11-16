#include "hzpch.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Hazel/Scene/Component.h"
#include <imgui.h>

namespace Hazel {

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> context)
	{
		SetContext(context);
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene> context)
	{
		m_Context = context;
	}
	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy Panel");

		auto view = m_Context->GetRegistry().view<TagComponent>();

		for (auto& entity : view) {
			Entity target{ entity,  m_Context->Raw() };
			DrawEntityNode(target);
		}

		ImGui::End();
	}
	void SceneHierarchyPanel::DrawEntityNode(Entity& entity) {

		auto& tag = entity.GetComponent<TagComponent>();
		ImGuiTreeNodeFlags flags = (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, tag.tag.c_str());

		if (ImGui::IsItemClicked()) {
			m_SelectedEntity = entity;
		}

		if (opened) {
			ImGui::TreePop();
		}
	}
}