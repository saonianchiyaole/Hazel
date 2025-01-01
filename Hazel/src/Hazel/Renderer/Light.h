#pragma once

#include "glm/glm.hpp"


namespace Hazel {


	enum class LightType {
		None = 0, PointLight, DirectionalLight, SpotLight, AreaLight
	};

	// Use positive Z as the light direction

	class Light {
		glm::vec3 m_Radiance;
		LightType m_Type;

		Light() {

		}
	};

	class PointLight {

	};

	class DirectionalLight {
		glm::vec3 m_Direction;
		glm::vec3 color;
	};

	class SpotLight {

	};


	class AreaLight {

	};


	// TODO
	/*class SkyBoxLight {

	};*/
}