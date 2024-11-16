#pragma once


#include "Hazel/Core/Core.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel {
	
	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene> context);
		void SetContext(const Ref<Scene> context);

		void OnImGuiRender();

		void DrawEntityNode(Entity& entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectedEntity;
	};
}