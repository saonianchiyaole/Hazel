#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/ShaderUniform.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	class Material {
	public:

		Material() = default;
		Material(Ref<Shader> shader);
		~Material();

		template<typename T>
		inline T* GetData(std::string name)
		{
			if (m_Data.find(name) != m_Data.end() && m_Data[name] != nullptr)
			{
				return (T*)m_Data[name];
			}
			return nullptr;
		}

		template<typename T>
		inline void SetData(std::string name, T data)
		{
			if (m_Data.find(name) == m_Data.end())
				return;

			for (const auto& uniform : m_Shader->GetUniforms()) {
				if (uniform->GetName() == name && Utils::isDataFormatCorrect<T>(uniform->GetType()))
				{
					*(T*)m_Data[name] = data;

					break;
				}
			}

		}

		template<typename T>
		inline void SetData(std::string name, T* data)
		{
			if (m_Data.find(name) == m_Data.end())
				return;

			for (const auto& uniform : m_Shader->GetUniforms()) {
				if (uniform->GetName() == name && Utils::isDataFormatCorrect<T>(uniform->GetType()))
				{
					*(T*)m_Data[name] = *data;
					break;
				}
			}

		}

		template<typename T>
		inline void SetData(std::string name, Ref<T> data)
		{
			if (m_Data.find(name) == m_Data.end())
				return;

			for (const auto& uniform : m_Shader->GetUniforms()) {
				if (uniform->GetName() == name && Utils::isDataFormatCorrect<T>(uniform->GetType()))
				{
					*(T*)m_Data[name] = *data.get();
					break;
				}
			}

		}

		template<>
		inline void SetData(std::string name, Ref<Texture2D> data)
		{
			if (m_Data.find(name) == m_Data.end())
				return;

			for (const auto& uniform : m_Shader->GetUniforms()) {
				if (uniform->GetName() == name && Utils::isDataFormatCorrect<Texture2D>(uniform->GetType()))
				{
					m_Data[name] = data.get();
					break;
				}
			}

		}


		void Submit();

		Ref<Shader> GetShader();

		void SetShader(Ref<Shader> shader);
		std::string GetName();
		void SetName(std::string name);

	private:

		void FreeMemory();

	private:

		std::string m_Name;
		Ref<Shader> m_Shader;

		std::unordered_map<std::string, void*> m_Data;

	};

}