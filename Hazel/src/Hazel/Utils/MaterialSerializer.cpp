#include "hzpch.h"
#include "Hazel/Utils/MaterialSerializer.h"

#include <fstream>

#include "Hazel/Utils/YAML.h"

namespace Hazel {

	namespace Utils {


		static std::unordered_map<std::string, Ref<ShaderUniform>> s_NameToShaderUniform;


		bool MaterialSerializer::Serialize(const std::string& filepath)
		{
			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Material" << YAML::Value << m_Material->GetName();
			out << YAML::Key << "Shader" << YAML::Value << m_Material->GetShader()->GetPath();
			out << YAML::Key << "Uniforms" << YAML::BeginSeq;

			for (const auto& uniform : m_Material->GetShader()->GetUniforms()) {
				void* data = m_Material->GetData<void*>(uniform->GetName());
				if (data)
					SerializeUniform(out, uniform, data);
			}

			out << YAML::EndSeq;
			out << YAML::EndMap;

			std::ofstream fout(filepath);
			fout << out.c_str();

			return true;
		}

		bool MaterialSerializer::Deserialize(const std::string& filepath)
		{
			s_NameToShaderUniform.clear();

			std::ifstream fin(filepath);
			std::stringstream strStream;
			strStream << fin.rdbuf();

			YAML::Node data = YAML::Load(strStream.str());
			if (!data["Material"])
				return false;

			//Shader
			std::string shaderPath = data["Shader"].as<std::string>();
			Ref<Shader> shader = ShaderLibrary::Load(shaderPath);

			m_Material->SetPath(filepath);
			//Set Data
			m_Material->SetShader(shader);

			//Uniforms
			for (auto uniform : shader->GetUniforms()) {
				s_NameToShaderUniform[uniform->GetName()] = uniform;
			}




			HZ_CORE_TRACE("Deserializing material '{0}'", data["Material"].as<std::string>());


			auto uniforms = data["Uniforms"];
			if (!uniforms)
				return false;


			for (auto uniform : uniforms)
			{

				auto it = uniform.begin();
				std::string shaderUniformName = it->first.as<std::string>();

				Ref<ShaderUniform> shaderUniform = s_NameToShaderUniform[shaderUniformName];
				if (!shaderUniform)
					continue;
				//m_Material->SetData(key, uniform[key].as<void*>());*/
				if (!uniform[shaderUniformName])
					continue;

				switch (shaderUniform->GetType())
				{
				case ShaderDataType::None:
					break;
				case ShaderDataType::Float:
				{
					float data = uniform[shaderUniformName].as<float>();
					m_Material->SetData(shaderUniform->GetName(), data);
					break;
				}
				case ShaderDataType::Float2:
				case ShaderDataType::Vec2:
				{
					glm::vec2 data = uniform[shaderUniformName].as<glm::vec2>();
					m_Material->SetData(shaderUniform->GetName(), data);
					break;
				}
				case ShaderDataType::Float3:
				case ShaderDataType::Vec3:
				{
					glm::vec3 data = uniform[shaderUniformName].as<glm::vec3>();
					m_Material->SetData(shaderUniform->GetName(), data);
					break;
				}
				case ShaderDataType::Float4:
				case ShaderDataType::Vec4:
				{
					glm::vec4 data = uniform[shaderUniformName].as<glm::vec4>();
					m_Material->SetData(shaderUniform->GetName(), data);
					break;
				}
				case ShaderDataType::Sampler2D:

				{
					std::string path = uniform[shaderUniformName].as<std::string>();
					TextureLibrary::Load(path);
					Ref<Texture2D> texture = TextureLibrary::Get(path);
					m_Material->SetData(shaderUniform->GetName(), texture);
				}
				break;
				case ShaderDataType::Int:
				{
					int data = uniform[shaderUniformName].as<int>();
					m_Material->SetData(shaderUniform->GetName(), data);
					break;
				}
				case ShaderDataType::Bool:
				{
					bool data = uniform[shaderUniformName].as<bool>();
					m_Material->SetData(shaderUniform->GetName(), data);
					break;
				}
				default:
					break;
				}


			}
		}

		bool MaterialSerializer::SerializeUniform(YAML::Emitter& out, Ref<ShaderUniform> shaderUniform, void* data)
		{
			out << YAML::BeginMap;

			switch (shaderUniform->GetType())
			{
			case ShaderDataType::None:
				break;
			case ShaderDataType::Float:
				out << YAML::Key << shaderUniform->GetName() << YAML::Value << *(float*)data;
				break;
			case ShaderDataType::Float2:
			case ShaderDataType::Vec2:
				out << YAML::Key << shaderUniform->GetName() << YAML::Value << *(glm::vec2*)data;
				break;
			case ShaderDataType::Float3:
			case ShaderDataType::Vec3:
				out << YAML::Key << shaderUniform->GetName() << YAML::Value << *(glm::vec3*)data;
				break;
			case ShaderDataType::Float4:
			case ShaderDataType::Vec4:
				out << YAML::Key << shaderUniform->GetName() << YAML::Value << *(glm::vec4*)data;
				break;
			case ShaderDataType::Sampler2D:
				out << YAML::Key << shaderUniform->GetName() << YAML::Value << ((Texture2D*)data)->GetPath();
				break;
			case ShaderDataType::Int:
				out << YAML::Key << shaderUniform->GetName() << YAML::Value << *(int*)data;
				break;
			case ShaderDataType::Bool:
				out << YAML::Key << shaderUniform->GetName() << YAML::Value << *(bool*)data;
				break;
			default:
				break;
			}


			out << YAML::EndMap;
			return true;
		}

		void MaterialSerializer::DeserializeUniform(Ref<ShaderUniform> shaderUniform, void* data)
		{


		}

	}

}