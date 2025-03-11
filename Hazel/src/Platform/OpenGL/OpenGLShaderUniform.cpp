#include "hzpch.h"


#include "Platform/OpenGL/OpenGLShaderUniform.h"

#include "Platform/OpenGL/OpenGLTexture.h"

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Renderer/Renderer.h"

namespace Hazel {

	namespace Utils {

		int32_t GetOpenGLUniformTypeFromShaderDataType(ShaderDataType shaderDataType) {

			switch (shaderDataType)
			{
			case Hazel::ShaderDataType::None:
				return GL_ID_UNKNOWN;
			case Hazel::ShaderDataType::Float:
				return GL_FLOAT;
			case Hazel::ShaderDataType::Float2:
				return GL_FLOAT_VEC2;
			case Hazel::ShaderDataType::Float3:
				return GL_FLOAT_VEC3;
			case Hazel::ShaderDataType::Float4:
				return GL_FLOAT_VEC4;
			case Hazel::ShaderDataType::Vec2:
				return GL_FLOAT_VEC2;
			case Hazel::ShaderDataType::Vec3:
				return GL_FLOAT_VEC3;
			case Hazel::ShaderDataType::Vec4:
				return GL_FLOAT_VEC4;
			case Hazel::ShaderDataType::Mat3:
				return GL_FLOAT_MAT3;
			case Hazel::ShaderDataType::Mat4:
				return GL_FLOAT_MAT4;
			case Hazel::ShaderDataType::Int:
				return GL_INT;
			case Hazel::ShaderDataType::Int2:
				return GL_INT_VEC2;
			case Hazel::ShaderDataType::Int3:
				return GL_INT_VEC3;
			case Hazel::ShaderDataType::Int4:
				return GL_INT_VEC4;
			case Hazel::ShaderDataType::Bool:
				return GL_BOOL;
			case Hazel::ShaderDataType::Sampler2D:
				return GL_SAMPLER_2D;
			default:
				break;
			}

		}


	}


	OpenGLShaderUniform::OpenGLShaderUniform(std::string name, ShaderDataType type)
		: ShaderUniform()
	{
		m_Name = name;
		m_Type = type;
		m_OpenGLType = Utils::GetOpenGLUniformTypeFromShaderDataType(type);

	}




	void OpenGLShaderUniform::Submit(GLint shaderID, void* data)
	{

		GLint location = glGetUniformLocation(shaderID, m_Name.c_str());


		switch (m_OpenGLType) {
		case GL_FLOAT: {
			glUniform1f(location, *(float*)data);
			break;
		}
		case GL_FLOAT_VEC2: {
			glm::vec2 value = *(glm::vec2*)data;
			glUniform2fv(location, 1, glm::value_ptr(value));
			break;
		}
		case GL_FLOAT_VEC3: {
			glm::vec3 value = *(glm::vec3*)data;
			glUniform3fv(location, 1, glm::value_ptr(value));
			break;
		}
		case GL_FLOAT_VEC4: {
			glm::vec4 value = *(glm::vec4*)data;
			glUniform4fv(location, 1, glm::value_ptr(value));
			break;
		}
		case GL_FLOAT_MAT3: {
			glm::mat3 value = *(glm::mat3*)(data);
			glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
			break;
		}
		case GL_FLOAT_MAT4: {
			glm::mat4 value = *(glm::mat4*)(data);
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
			break;
		}
		case GL_INT: {
			int value = *(int*)(data);
			glUniform1i(location, value);
			break;
		}
		case GL_BOOL: {
			bool value = *(bool*)data ? 1 : 0;
			glUniform1i(location, value);
			break;
		}
		case GL_SAMPLER_2D: {
			if (!data)
				break;
			OpenGLTexture2D* texture = static_cast<OpenGLTexture2D*>(data);
			if (!texture->IsLoaded())
				break;
			uint8_t slot = texture->GetSlot();
			glUniform1i(location, slot);
			texture->Bind(slot);
			break;
		}
		case GL_SAMPLER_CUBE: {
			//int textureUnit = *static_cast<int*>(uniform->GetData());
			//glUniform1i(location, textureUnit); // 纹理单元绑定到 uniform
			break;
		}
		default:
			printf("Unsupported uniform type for '%s'.\n", m_Name);
			break;
		}
	}

}