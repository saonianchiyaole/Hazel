#include "hzpch.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel {
	Entity::Entity()
	{
		m_EntityHandle = entt::null;
		m_Scene = nullptr;
	}
	Entity::Entity(entt::entity entityHandle, Scene* scene)
		:m_EntityHandle(entityHandle), m_Scene(scene)
	{
		
	}

}