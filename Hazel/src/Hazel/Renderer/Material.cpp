#include "hzpch.h"

#include "Hazel/Renderer/Material.h"

namespace Hazel {


	Material::Material(Ref<Shader> shader)
	{
		m_Shader = shader;
		m_Name = shader->GetName();

		for (auto uniform : m_Shader->GetUniforms()) {
			m_Data[uniform->GetName()] = Utils::AllocateMemoryByShaderDataType(uniform->GetType());
		}
	}

	Material::~Material()
	{
		FreeMemory();
	}

	void Material::FreeMemory()
	{

		for (const auto& uniform : m_Shader->GetUniforms())
		{
			ShaderDataType type = uniform->GetType();
			void* data = m_Data[uniform->GetName()];

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
	}


	void Material::Submit()
	{
		m_Shader->Submit(m_Data);
	}

	Ref<Shader> Material::GetShader()
	{
		return m_Shader;
	}

	void Material::SetShader(Ref<Shader> shader)
	{
		if (!m_Shader || m_Shader->GetPath() != shader->GetPath())
		{
			m_Shader = shader;
			m_Data.clear();


			for (auto uniform : m_Shader->GetUniforms()) {
				m_Data[uniform->GetName()] = Utils::AllocateMemoryByShaderDataType(uniform->GetType());
			}
		}
	}

	std::string Material::GetName()
	{
		return m_Name;
	}

	void Material::SetName(std::string name)
	{
		m_Name = name;
	}




}