#pragma once

#include "Hazel/Scene/Scene.h"


namespace Hazel {
	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity entityHandle, Scene* scene);
		Entity(const Entity& other) = default;

		entt::entity GetHandle() {
			return this->m_EntityHandle;
		}

		template<class T, class... Args>
		T& AddComponent(Args&&... args) {
			HZ_CORE_ASSERT(!HasComponent<T>(), "Entity already has this component!")
			return m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<class T>
		T& GetComponent() {
			
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity dose not have this component!")
			return m_Scene->GetRegistry().get<T>(m_EntityHandle);
		}

		template<class T>
		bool HasComponent() {
			return m_Scene->GetRegistry().all_of<T>(m_EntityHandle);
		}

		template<class T>
		T RemoveComponent() {
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity dose not have this component!")
			return m_Scene->GetRegistry().remove<T>(m_EntityHandle);
		}

		bool operator ==(Entity& other) {
			return (m_EntityHandle == other.m_EntityHandle) && (m_Scene == other.m_Scene);
		}

		bool operator !=(Entity& other) {
			return !(*this == other);
		}

		

	private:
		entt::entity m_EntityHandle{ 0 };
		Scene* m_Scene = nullptr;
	};

	
}