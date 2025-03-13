#include "hzpch.h"

#include "Hazel/Renderer/Material.h"

#include "Hazel/Utils/MaterialSerializer.h"
#include "Hazel/Renderer/Renderer.h"

namespace Hazel {


	Material::Material(Ref<Shader> shader)
	{
		SetShader(shader);
		m_Name = shader->GetName();

	}

	Material::~Material()
	{
		//FreeMemory();

		m_Shader->DeleteAssocitaedMaterial(this);
	}

	void Material::FreeMemory()
	{
		for (auto& it : m_Data)
		{
			std::string name = it.first;
			//Buffer buffer = it.second;
			it.second.Free();
		}
	}


	void Material::Submit()
	{
		SetTexturesSlot();

		m_Shader->Submit(m_Data);
	}

	void Material::SetTexturesSlot()
	{
		for (auto it : m_NameToTextureAndSlot) {
			auto [texture, slot] = it.second;
			texture->SetSlot(slot);
		}
	}

	Ref<Shader> Material::GetShader()
	{
		return m_Shader;
	}

	void Material::ReloadShader()
	{
		//m_Data Check

		for (auto it = m_Data.begin(); it != m_Data.end(); ) {
			if (!m_Shader->GetUniform(it->first)) {
				it = m_Data.erase(it);  
			}
			else {
				++it;  
			}
		}


		//m_Shader

		for (auto& uniform : m_Shader->GetUniforms()) {

			if (m_Data.find(uniform->GetName()) == m_Data.end()) {
				m_Data[uniform->GetName()].Allocate(Utils::GetAllocatedMemoryByShaderDataType(uniform->GetType()));
				m_Data[uniform->GetName()].ZeroInitialize();
			}
		}

	}

	void Material::SetShader(Ref<Shader> shader)
	{
		if (!m_Shader || m_Shader->GetPath() != shader->GetPath())
		{
			m_Shader = shader;
			FreeMemory();
			m_Data.clear();
			m_NameToTextureAndSlot.clear();

			for (auto uniform : m_Shader->GetUniforms()) {
				//m_Data[uniform->GetName()] = Utils::AllocateMemoryByShaderDataType(uniform->GetType());
				m_Data[uniform->GetName()].Allocate(Utils::GetAllocatedMemoryByShaderDataType(uniform->GetType()));
				m_Data[uniform->GetName()].ZeroInitialize();
			}

			//deal with texture
			for (auto uniform : m_Shader->GetUniforms()) {
				if (uniform->GetType() == ShaderDataType::Sampler2D) {
					m_Data[uniform->GetName()].data = Renderer::GetDefaultBlackQuadTexture().get();
					m_NameToTextureAndSlot[uniform->GetName()] = std::pair(Renderer::GetDefaultBlackQuadTexture(), m_NameToTextureAndSlot.size());
				}
			}

			shader->AddAssocitaedMaterial(this);

			m_Flag = shader->GetFlag();
		}
	}

	void Material::SetPath(const std::string& path)
	{
		m_Path = path;
	}

	std::string Material::GetName()
	{
		return m_Name;
	}

	void Material::SetName(const std::string& name)
	{
		m_Name = name;
	}

	uint32_t Material::GetSampleUniformAmount()
	{
		return m_NameToTextureAndSlot.size();
	}

	Ref<Material> Material::Create(std::filesystem::path filepath)
	{
		if (!std::filesystem::exists(filepath))
			return nullptr;

		Ref<Material> material = MakeRef<Material>();

		Utils::MaterialSerializer materialSerializer(material);
		materialSerializer.Deserialize(filepath.string());

		return material;
	}

	Ref<Material> Material::Create()
	{
		return MakeRef<Material>();
	}


}