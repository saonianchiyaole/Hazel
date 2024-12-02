#pragma once


#include "glm/glm.hpp"

namespace Hazel {

	namespace Math {

		bool Decompose(const glm::mat4& transform, glm::vec3& translate, glm::vec3& rotation, glm::vec3& scale);

	}

}

