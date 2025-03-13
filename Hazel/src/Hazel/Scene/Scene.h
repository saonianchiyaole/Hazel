#pragma once

#include "entt.hpp"

#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Core/Timestep.h"
#include "glm/glm.hpp"
#include "Hazel/Core/UUID.h"


class b2World;

namespace Hazel {

	enum class SceneState {
		Edit = 0,
		Play = 1
	};
	
	
	class TextureCube;
	class Entity;
	class Environment;
	class Scene {
	public:


		static Ref<Scene> CopyScene(Ref<Scene> src);

		template<class Component>
		static void CopyComponent(Ref<Scene> src, Ref<Scene> dst, UUID uuid);

		//copy entity in the same scene
		template<class Component>
		void CopyComponent(Entity src, Entity dst) {
			if (src.HasComponent<Component>()) {
				m_Registry.emplace_or_replace<Component>(dst.GetHandle(), src.GetComponent<Component>());
			}
		}

		Entity CreateEntity(const std::string& name);
		Entity CreateEnttiyWithUUID(const std::string& name,const uint64_t ID);
		Entity GetEntityFormUUID(UUID uuid);
		
		Entity FindEntityByName(std::string name);

		Entity DulpicateEntity(Entity entity);

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

		void UpdataAnimation(Timestep ts);
		
		template<typename ...Component>
		auto GetAllEntityWith(){
			return m_Registry.view<Component...>();
		}

		Ref<Environment> GetEnvironment();
		
		
		void SetViewPortSize(glm::vec2 viewPortSize);
		
		void SetSkybox(Ref<TextureCube> skybox);
		void SetSkybox(Ref<Environment> environment);
		void SetEnvironment(Ref<Environment> environment);

		Ref<TextureCube> GetSkybox();


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

		Ref<TextureCube> m_Skybox;
		Ref<Environment> m_Environment;

		std::unordered_map<UUID, entt::entity> m_UUIDToEntity;

		SceneState m_State;
		friend class Entity;
		friend class Editor;
	};

}