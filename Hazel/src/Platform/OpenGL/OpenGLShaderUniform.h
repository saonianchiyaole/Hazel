#pragma once

#include "Hazel/Renderer/ShaderUniform.h"
#include "glad/glad.h"

namespace Hazel {


	class OpenGLShaderUniform : public ShaderUniform {
	public:
		OpenGLShaderUniform(std::string name, ShaderDataType type);

		virtual void Submit(GLint shaderID) override;
	private:
		int32_t m_OpenGLType;
	};

}
