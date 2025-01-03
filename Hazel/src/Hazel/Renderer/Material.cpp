#include "hzpch.h"

#include "Hazel/Renderer/Material.h"

namespace Hazel {


	Material::Material(Ref<Shader> shader)
	{
		m_Shader = shader;
		m_Name = shader->GetName();
	}

	void Material::Submit()
	{
		m_Shader->Submit();
	}

	Ref<Shader> Material::GetShader()
	{
		return m_Shader;
	}

	void Material::SetShader(Ref<Shader> shader)
	{
		m_Shader = shader;
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