#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/ShaderUniform.h"


namespace Hazel {

	class Material {
	public:

		Material() = default;
		Material(Ref<Shader> shader);


		void Submit();

	private:

		Ref<Shader> m_Shader;

	};

}