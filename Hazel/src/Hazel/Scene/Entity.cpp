#include "hzpch.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel {

	Entity::Entity(entt::entity entityHandle, Scene* scene)
		:m_EntityHandle(entityHandle), m_Scene(scene)
	{

	}

}