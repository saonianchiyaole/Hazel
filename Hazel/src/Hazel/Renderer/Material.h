#pragma once

#include "Hazel/Renderer/Shader.h"


namespace Hazel {

	class Material {
	public:

		Material() = default;
		Material(Ref<Shader> shader);
	private:

		Ref<Shader> m_Shader;
	};

}