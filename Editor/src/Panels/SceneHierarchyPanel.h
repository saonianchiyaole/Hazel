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
		void DrawComponents(Entity& entity);

		inline Entity GetSelectedEntity() const{
			return m_SelectedEntity;
		}


	private:
		Ref<Scene> m_Context;
		Entity m_SelectedEntity;
	};
}