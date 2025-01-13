#include "hzpch.h"

#include "Hazel/Renderer/ShaderUniform.h"

#include "Hazel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLShaderUniform.h"
#include "Platform/OpenGL/OpenGLTexture.h"


#include "glm/glm.hpp"

namespace Hazel {


	namespace Utils {





		void* AllocateMemoryByShaderDataType(ShaderDataType type, size_t amount) {
			switch (type)
			{
			case Hazel::ShaderDataType::None:
				break;
			case Hazel::ShaderDataType::Float:
				return new float();
			case Hazel::ShaderDataType::Float2:
				return new glm::vec2();
			case Hazel::ShaderDataType::Float3:
				return new glm::vec3();
			case Hazel::ShaderDataType::Float4:
				return new glm::vec4();
			case Hazel::ShaderDataType::Vec2:
				return new glm::vec2();
			case Hazel::ShaderDataType::Vec3:
				return new glm::vec3();
			case Hazel::ShaderDataType::Vec4:
				return new glm::vec4();
			case Hazel::ShaderDataType::Mat3:
				return new glm::mat3();
			case Hazel::ShaderDataType::Mat4:
				return new glm::mat4();
			case Hazel::ShaderDataType::Int:
				return new glm::int32_t();
			case Hazel::ShaderDataType::Int2:
				return new glm::vec2();
			case Hazel::ShaderDataType::Int3:
				return new glm::vec3();
			case Hazel::ShaderDataType::Int4:
				return new glm::vec4();
			case Hazel::ShaderDataType::Bool:
				return new bool();
			case Hazel::ShaderDataType::Sampler2D:
			{
				return nullptr;
			}
			default:
				break;
			}
		}

		
	}



	ShaderDataType ShaderUniform::GetType()
	{
		return m_Type;
	}

	std::string ShaderUniform::GetName()
	{
		return m_Name;
	}

	Ref<ShaderUniform> ShaderUniform::Create(const std::string name, ShaderDataType type)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShaderUniform>(name, type);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}





	Hazel::ShaderUniform::~ShaderUniform()
	{
		//FreeMemory();
	}

	

	ShaderUniform::ShaderUniform()
	{

	}

	ShaderUniform::ShaderUniform(std::string name, ShaderDataType type)
	{

		m_Name = name;
	}

	





}