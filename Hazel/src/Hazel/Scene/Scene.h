#pragma once

#include "entt.hpp"

#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Core/Timestep.h"
#include "glm/glm.hpp"

class b2World;

namespace Hazel {

	enum class SceneState {
		Edit = 0,
		Play = 1
	};

	class Entity;
	class Scene {
	public:

		Entity CreateEntity(const std::string& name);
		void DestroyEntity(Entity entity);

		inline entt::registry& GetRegistry() {
			return m_Registry;
		}

		Ref<Camera> GetPrimaryCamera();

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void OnUpdateEditor(Timestep ts, const EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts);

		SceneState GetState() {
			return m_State;
		}

		Scene* Raw();
	private:
		template<class T>
		void OnAddComponent(Entity& entity, T& component);
	private:
		entt::registry m_Registry;
		glm::vec2 m_ViewPortSize;

		b2World* m_PhysicsWorld;

		SceneState m_State;
		friend class Entity;
		friend class Editor;
	};

}