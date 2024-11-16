#pragma once
#include "Hazel/Scene/Entity.h"


namespace Hazel {

	class ScriptableEntity {
	public:
		template<class T>
		T& GetComponent() {
			return m_Entity.GetComponent<T>();
		}
	protected:
		virtual void OnCreate() {};
		virtual void OnDestroy() {};
		virtual void OnUpdate(Timestep ts) {};
	private:
		friend class Scene;
		Entity m_Entity;
	};

}