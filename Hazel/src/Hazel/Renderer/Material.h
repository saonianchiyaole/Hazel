#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/ShaderUniform.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Core/Buffer.h"
#include "Hazel/Asset/Asset.h"

namespace Hazel {


	namespace Utils {

		template<typename T>
		bool IsDataFormatCorrect(ShaderReflectionData& data) {

			switch (data.type)
			{			
			case DescriptorType::Sampler2D:
			{
				return (std::is_same<T, Hazel::Texture2D*>::value ||
					std::is_same<T, Ref<Texture2D>>::value ||
					std::is_same<T, Hazel::Texture2D>::value) && data.size == 8;
			}
			case DescriptorType::SamplerCube:
				break;
			default:
				break;
			}
		}

		template<typename T>
		bool IsDataFormatCorrect(ShaderDataType type) {

			switch (type)
			{
			case Hazel::ShaderDataType::None:
				return false;
			case Hazel::ShaderDataType::Sampler2D:

			{
				return std::is_same<T, Hazel::Texture2D*>::value ||
					std::is_same<T, Ref<Texture2D>>::value ||
					std::is_same<T, Hazel::Texture2D>::value;
			}
			case Hazel::ShaderDataType::SamplerCube:
				break;
			default:
				break;
			}

			return sizeof(T) == Utils::ShaderDataTypeSize(type);
		}

	}

	class Material : public Asset {
	public:
		friend class MaterialSerializer;
		friend class Mesh;

		Material() = default;
		Material(Ref<Shader> shader);
		~Material();


		virtual AssetType GetAssetType() override {
			return AssetType::Material;
		}

		template<typename T>
		inline T* GetData(const std::string& name)
		{
			if (m_Data.find(name) != m_Data.end())
			{
				return m_Data[name].Read<T>();
			}
			return nullptr;
		}

		template<typename T>
		inline void SetData(const std::string& name, T data)
		{
			if (m_Data.find(name) == m_Data.end() || sizeof(T) != m_Shader->GetReflectionDataByName(name)->size)
				return;

			m_Data[name].Write(&data, sizeof(T));
		}

		template<typename T>
		inline void SetData(const std::string& name, T* data)
		{
			if (m_Data.find(name) == m_Data.end() || sizeof(T) != m_Shader->GetReflectionDataByName(name)->size)
				return;

			m_Data[name].Write(&data, sizeof(T));

		}

		template<typename T>
		inline void SetData(const std::string& name, Ref<T> data)
		{
			if (m_Data.find(name) == m_Data.end() || sizeof(T) != m_Shader->GetReflectionDataByName(name)->size)
				return;

			m_Data[name].Write(&data.get(), sizeof(T));

		}

		
		virtual void SetData(const std::string& name, Ref<Texture2D> data)
		{
			if (m_Data.find(name) == m_Data.end() || !Utils::IsDataFormatCorrect<Texture2D>(*m_Shader->GetReflectionDataByName(name)))
				return;

			if (!m_Data[name] && m_NameToTextureAndSlot.find(name) == m_NameToTextureAndSlot.end()) {
				static size_t slotIndex = 0;
				m_NameToTextureAndSlot[name] = { data, slotIndex };
				slotIndex++;
			}
			else {
				m_NameToTextureAndSlot[name] = { data, m_NameToTextureAndSlot[name].second };
			}

			m_Data[name].Write(data.get());
		
		}


		virtual void Submit();

		Ref<Shader> GetShader();
		std::string GetName();
		uint32_t GetSampleUniformAmount();

		void ReloadShader();

		virtual void SetShader(Ref<Shader> shader);
		void SetName(const std::string& name);
		void SetPath(const std::string& path);

		void SetTexturesSlot();

		bool IsFormMesh() { return m_IsFromMesh; }

		static Ref<Material> Create(std::filesystem::path filepath);
		static Ref<Material> Create();

	private:

		void FreeMemory();

	protected:

		std::string m_Name = "Main";
		Ref<Shader> m_Shader;
		std::string m_Path;

		bool m_UseAlbedoTex = false;
		bool m_UseNormalTex = false;
		bool m_UseRoughnessTex = false;
		bool m_UseMetalnessTex = false;


		std::unordered_map<std::string, std::pair<Ref<Texture2D>, uint32_t>> m_NameToTextureAndSlot;

		std::unordered_map<std::string, Buffer> m_Data;

		friend class MaterialSerializer;
		bool m_IsFromMesh = false;
	};

	class MaterialTable {
	public:

	private:
	};

}