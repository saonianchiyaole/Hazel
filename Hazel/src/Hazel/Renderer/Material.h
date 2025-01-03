#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/ShaderUniform.h"


namespace Hazel {

	class Material {
	public:

		Material() = default;
		Material(Ref<Shader> shader);


		void Submit();

		Ref<Shader> GetShader();

		void SetShader(Ref<Shader> shader);
		std::string GetName();
		void SetName(std::string name);

	private:
		std::string m_Name;
		Ref<Shader> m_Shader;

	};

}