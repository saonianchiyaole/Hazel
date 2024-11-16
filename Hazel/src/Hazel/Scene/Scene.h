#pragma once

#include "entt.hpp"

#include "Hazel/Core/Timestep.h"

namespace Hazel {
	class Entity;
	class Scene {
	public:

		Entity CreateEntity(const std::string& name);

		entt::registry& GetRegistry(){
			return m_Registry;
		}

		void OnUpdate(Timestep ts);

		Scene* Raw();
	private:
		entt::registry m_Registry;


	};

}