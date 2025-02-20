#include "hzpch.h"

#include "Hazel/Renderer/ShaderUniform.h"

#include "Hazel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLShaderUniform.h"
#include "Platform/OpenGL/OpenGLTexture.h"


#include "glm/glm.hpp"

namespace Hazel {


	namespace Utils {



		void FreeMemoryByShaderDataType(ShaderDataType type, void* data) {
			switch (type)
			{
			case Hazel::ShaderDataType::None:
				break;
			case Hazel::ShaderDataType::Float:
			{
				float* ptr = (float*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Float2:
			{
				glm::vec2* ptr = (glm::vec2*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Float3:
			{
				glm::vec3* ptr = (glm::vec3*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Float4:
			{
				glm::vec4* ptr = (glm::vec4*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Vec2:
			{
				glm::vec2* ptr = (glm::vec2*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Vec3:
			{
				glm::vec3* ptr = (glm::vec3*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Vec4:
			{
				glm::vec4* ptr = (glm::vec4*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Mat3:
			{
				glm::mat3* ptr = (glm::mat3*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Mat4:
			{
				glm::mat4* ptr = (glm::mat4*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Int:
			{
				int32_t* ptr = (int32_t*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Int2:
			{
				glm::vec2* ptr = (glm::vec2*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Int3:
			{
				glm::vec3* ptr = (glm::vec3*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Int4:
			{
				glm::vec4* ptr = (glm::vec4*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Bool:
			{
				bool* ptr = (bool*)data;
				delete ptr;
				break;
			}
			case Hazel::ShaderDataType::Sampler2D:
			{
				//Do nothing, Let Texture library handle this
			}
			default:
				break;
			}
		}

		uint64_t GetAllocatedMemoryByShaderDataType(ShaderDataType type)
		{
			switch (type)
			{
			case Hazel::ShaderDataType::None:
				break;
			case Hazel::ShaderDataType::Float:
				return sizeof(float);
			case Hazel::ShaderDataType::Float2:
				return sizeof(glm::vec2);
			case Hazel::ShaderDataType::Float3:
				return sizeof(glm::vec3);
			case Hazel::ShaderDataType::Float4:
				return sizeof(glm::vec4);
			case Hazel::ShaderDataType::Vec2:
				return sizeof(glm::vec2);
			case Hazel::ShaderDataType::Vec3:
				return sizeof(glm::vec3);
			case Hazel::ShaderDataType::Vec4:
				return sizeof(glm::vec4);
			case Hazel::ShaderDataType::Mat3:
				return sizeof(glm::mat3);
			case Hazel::ShaderDataType::Mat4:
				return sizeof(glm::mat4);
			case Hazel::ShaderDataType::Int:
				return sizeof(int);
			case Hazel::ShaderDataType::Int2:
				return sizeof(glm::vec2);
			case Hazel::ShaderDataType::Int3:
				return sizeof(glm::vec3);
			case Hazel::ShaderDataType::Int4:
				return sizeof(glm::vec4);
			case Hazel::ShaderDataType::Bool:
				return sizeof(bool);
			case Hazel::ShaderDataType::Sampler2D:
			{
				return 0;
			}
			default:
				break;
			}
		}
		

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