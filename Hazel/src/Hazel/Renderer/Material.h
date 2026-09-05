#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/ShaderUniform.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Core/Buffer.h"
#include "Hazel/Resource/Resource.h"
#include "Hazel/Resource/ResourceManager.h"

namespace Hazel {


	typedef std::unordered_map<std::string, Buffer> MaterialDataMap;

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

	class Material {
	public:
		friend class MaterialSerializer;
		friend class Mesh;

		Material() = default;
		Material(Ref<Shader> shader);
		~Material();


		virtual ResourceType GetResourceType() {
			return ResourceType::Material;
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
		inline bool SetData(const std::string& name, T data, uint32_t index = 0)
		{
			Ref<Shader> shader = ResourceManager<Shader>::Get(m_Shader);
			if (!shader || m_Data.find(name) == m_Data.end() || sizeof(T) != shader->GetReflectionDataByName(name)->size)
				return false;

			m_Data[name].Write(&data, sizeof(T));
			return true;
		}

		template<typename T>
		inline bool SetData(const std::string& name, T* data, uint32_t index = 0)
		{
			Ref<Shader> shader = ResourceManager<Shader>::Get(m_Shader);
			if (!shader || m_Data.find(name) == m_Data.end() || sizeof(T) != shader->GetReflectionDataByName(name)->size)
				return false;

			m_Data[name].Write(data, sizeof(T));
			return true;
		}

		template<typename T>
		inline bool SetData(const std::string& name, Ref<T> data, uint32_t index = 0)
		{
			Ref<Shader> shader = ResourceManager<Shader>::Get(m_Shader);
			if (!shader || m_Data.find(name) == m_Data.end() || sizeof(T) != shader->GetReflectionDataByName(name)->size)
				return false;

			m_Data[name].Write(data.get(), sizeof(T));
			return true;

		}

		
		virtual bool SetData(const std::string& name, const Handle<Texture2D>& data, uint32_t index = 0)
		{
			Ref<Shader> shader = ResourceManager<Shader>::Get(m_Shader);
			Ref<Texture2D> texture = ResourceManager<Texture2D>::Get(data);

			if (!shader || !texture) {
				return false;
			}

			if (m_Data.find(name) == m_Data.end() || !Utils::IsDataFormatCorrect<Texture2D>(*shader->GetReflectionDataByName(name)))
				return false;

			if (!m_Data[name] && m_NameToTextureAndSlot.find(name) == m_NameToTextureAndSlot.end()) {
				m_NameToTextureAndSlot[name] = { data, index };
			}
			else {
				m_NameToTextureAndSlot[name] = { data, m_NameToTextureAndSlot[name].second };
			}

			m_Data[name].Write(texture.get());
		
			return true;
		}

		virtual bool SetData(const std::string& name, Ref<Texture2D> data, uint32_t index = 0) {
			if (!data) {
				return false;
			}
			return SetData(name, ResourceManager<Texture2D>::Add(data), index);
		}


		virtual void Submit();

		Handle<Shader>	GetShader();
		std::string		GetName();
		uint32_t		GetSampleUniformAmount();

		void			ReloadShader();

		void			SetShader	(Handle<Shader> shader);
		virtual void	SetShader	(Ref<Shader> shader);

		void			SetName		(const std::string& name);
		void			SetPath		(const std::string& path);

		void			SetTexturesSlot();

		bool			IsFormMesh() { return m_IsFromMesh; }

		static Ref<Material> Create(std::filesystem::path filepath);
		static Ref<Material> Create();

	private:

		void FreeMemory();

	protected:

		std::string m_Name = "Main";
		std::string m_Path;
		Handle<Shader> m_Shader;

		bool m_UseAlbedoTex = false;
		bool m_UseNormalTex = false;
		bool m_UseRoughnessTex = false;
		bool m_UseMetalnessTex = false;


		std::unordered_map<std::string, std::pair<Handle<Texture2D>, uint32_t>> m_NameToTextureAndSlot;

		MaterialDataMap m_Data;

		friend class MaterialSerializer;
		bool m_IsFromMesh = false;
	};

	class MaterialTable {
	public:

	private:
	};

}
