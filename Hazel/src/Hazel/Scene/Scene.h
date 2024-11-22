#pragma once

#include "entt.hpp"

#include "Hazel/Renderer/Camera.h"
#include "Hazel/Core/Timestep.h"
#include "glm/glm.hpp"

namespace Hazel {
	class Entity;
	class Scene {
	public:

		Entity CreateEntity(const std::string& name);
		void DestroyEntity(Entity entity);

		inline entt::registry& GetRegistry(){
			return m_Registry;
		}

		Ref<Camera> GetPrimaryCamera();


		void OnUpdate(Timestep ts);

		Scene* Raw();
	private:
		template<class T>
		void OnAddComponent(Entity& entity, T& component);
	private:
		entt::registry m_Registry;
		glm::vec2 m_ViewPortSize;

		friend class Entity;
		friend class Editor;
	};

}