#pragma once


#include "Hazel/Core/Core.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

#include "Hazel/Renderer/Texture.h"


namespace Hazel {
	
	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(const Ref<Scene> context);
		void SetContext(const Ref<Scene> context);
		void SetSelectedEntity(const entt::entity entity);

		void OnImGuiRender();

		void DrawEntityNode(Entity& entity);
		
		void DrawComponents(Entity& entity);
		template<class T, class Function>
		void DrawComponent(Entity& entity, std::string componentName, Function func);


		inline Entity GetSelectedEntity() const{
			return m_SelectedEntity;
		}


	private:
		Ref<Scene> m_Context;
		Entity m_SelectedEntity;
		Ref<Texture2D> m_EmptyTexture;
	};
}