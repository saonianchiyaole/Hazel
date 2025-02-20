#include "hzpch.h"

#include "Hazel/Renderer/Material.h"

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

			for (auto uniform : m_Shader->GetUniforms()) {
				//m_Data[uniform->GetName()] = Utils::AllocateMemoryByShaderDataType(uniform->GetType());
				m_Data[uniform->GetName()].Allocate(Utils::GetAllocatedMemoryByShaderDataType(uniform->GetType()));
				m_Data[uniform->GetName()].ZeroInitialize();
			}

			shader->AddAssocitaedMaterial(this);
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

	uint32_t Material::GetSampleUniformAmount()
	{
		return m_NameToTextureAndSlot.size();
	}


}