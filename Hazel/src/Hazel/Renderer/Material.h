#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/ShaderUniform.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Core/Buffer.h"

namespace Hazel {

	class Material {
	public:
		friend class MaterialSerializer;

		Material() = default;
		Material(Ref<Shader> shader);
		~Material();

		template<typename T>
		inline T* GetData(std::string name)
		{
			if (m_Data.find(name) != m_Data.end() && m_Data[name].data != nullptr)
			{
				return m_Data[name].Read<T>();
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
					m_Data[name].Write(&data, sizeof(T));

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
					m_Data[name].Write(data, sizeof(T));
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
					m_Data[name].Write(data.get(), sizeof(T));
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

					m_Data[name].data = data.get();
					break;
				}
			}

		}


		void Submit();

		Ref<Shader> GetShader();
		std::string GetName();
		uint32_t GetSampleUniformAmount();

		void ReloadShader();

		void SetShader(Ref<Shader> shader);
		void SetName(std::string& name);
		void SetPath(const std::string& path);

		void SetTexturesSlot();


		static Ref<Material> Create(std::filesystem::path filepath);
		static Ref<Material> Create();
	private:

		void FreeMemory();

	private:

		std::string m_Name;
		Ref<Shader> m_Shader;
		std::string m_Path;

		bool m_UseAlbedoTex = false;
		bool m_UseNormalTex = false;
		bool m_UseRoughnessTex = false;
		bool m_UseMetalnessTex = false;

		std::unordered_map<std::string, std::pair<Ref<Texture2D>, uint32_t>> m_NameToTextureAndSlot;
		//std::unordered_map<std::string, void*> m_Data;

		std::unordered_map<std::string, Buffer> m_Data;
	};

}