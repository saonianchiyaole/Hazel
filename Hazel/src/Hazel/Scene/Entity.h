#pragma once

#include "Hazel/Scene/Scene.h"


namespace Hazel {
	class Entity {
		friend class SceneSerializer;
	public:
		Entity();
		Entity(entt::entity entityHandle, Scene* scene);
		Entity(const Entity& other) = default;

		entt::entity GetHandle() {
			return this->m_EntityHandle;
		}

		template<class T, class... Args>
		T& AddComponent(Args&&... args) {
			HZ_CORE_ASSERT(!HasComponent<T>(), "Entity already has this component!");
			auto& component = m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnAddComponent(*this, component);
			return component;
		}

		template<class T>
		T& GetComponent() {

			HZ_CORE_ASSERT(HasComponent<T>(), "Entity dose not have this component!");
			return m_Scene->GetRegistry().get<T>(m_EntityHandle);
		}

		template<class T>
		bool HasComponent() {
			if (m_Scene)
				return m_Scene->GetRegistry().all_of<T>(m_EntityHandle);
			return false;
		}

		template<class T>
		T RemoveComponent() {
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity dose not have this component!");
			return m_Scene->GetRegistry().remove<T>(m_EntityHandle);
		}


		inline uint64_t GetEntityID() {
			return m_EntityID;
		}

		bool operator ==(Entity& other) {
			return (m_EntityHandle == other.m_EntityHandle) && (m_Scene == other.m_Scene);
		}

		bool operator !=(Entity& other) {
			return !(*this == other);
		}

		operator bool() {
			return m_EntityHandle != entt::null;
		}
		operator int() {
			return (int)m_EntityHandle;
		}
		operator entt::entity() {
			return m_EntityHandle;
		}


	private:
		entt::entity m_EntityHandle{ entt::null };
		uint64_t m_EntityID;
		Scene* m_Scene = nullptr;

		friend class Scene;
	};


}