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
					if (!m_Data[name] && m_NameToTextureAndSlot.find(name) == m_NameToTextureAndSlot.end()) {
						static size_t slotIndex = 0;
						m_NameToTextureAndSlot[name] = { data, slotIndex };
						slotIndex++;
					}
					else {
						m_NameToTextureAndSlot[name] = { data, m_NameToTextureAndSlot[name].second };
					}

					m_Data[name] = data.get();
					
				
					break;
				}
			}

		}


		void Submit();
		void SetTexturesSlot();


		Ref<Shader> GetShader();

		void SetShader(Ref<Shader> shader);
		std::string GetName();
		void SetName(std::string name);

	private:

		void FreeMemory();

	private:

		std::string m_Name;
		Ref<Shader> m_Shader;
		
		std::unordered_map<std::string, std::pair<Ref<Texture2D>, uint32_t>> m_NameToTextureAndSlot;
		std::unordered_map<std::string, void*> m_Data;

	};

}