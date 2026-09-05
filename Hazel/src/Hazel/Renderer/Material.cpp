#include "hzpch.h"

#include "Hazel/Renderer/Material.h"

#include "Hazel/Utils/MaterialSerializer.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Resource/ResourceManager.h"

namespace Hazel {


	Material::Material(Ref<Shader> shader)
	{
		SetShader(shader);
		m_Name = shader->GetName();

	}

	Material::~Material()
	{
		//FreeMemory();
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
		auto shader = ResourceManager<Shader>::Get(m_Shader);
		shader->Submit(m_Data);
	}

	void Material::SetTexturesSlot()
	{
		for (auto it : m_NameToTextureAndSlot) {
			auto [textureHandle, slot] = it.second;
			if (ResourceManager<Texture2D>::Has(textureHandle)) {
				Ref<Texture2D> texture = ResourceManager<Texture2D>::Get(textureHandle);
				texture->SetSlot(slot);
			}
		}
	}

	Handle<Shader> Material::GetShader()
	{
		return m_Shader;
	}

	void Material::ReloadShader()
	{
		//m_Data Check
		
		Ref<Shader> shader = ResourceManager<Shader>::Get(m_Shader);

		for (auto it = m_Data.begin(); it != m_Data.end(); ) {
			if (!shader->GetUniform(it->first)) {
				it = m_Data.erase(it);  
			}
			else {
				++it;  
			}
		}


		//m_Shader

		for (auto& uniform : shader->GetUniforms()) {

			if (m_Data.find(uniform->GetName()) == m_Data.end()) {
				m_Data[uniform->GetName()].Allocate(Utils::GetAllocatedMemoryByShaderDataType(uniform->GetType()));
				m_Data[uniform->GetName()].ZeroInitialize();
			}
		}

	}

	void Material::SetShader(Handle<Shader> shaderHandle)
	{
		Ref<Shader> shader = ResourceManager<Shader>::Get(shaderHandle);
		SetShader(shader);		
	}

	void Material::SetShader(Ref<Shader> shader) {
		
		if (shader)
		{

			Handle<Shader> shaderHandle = ResourceManager<Shader>::Add(shader);
			m_Shader = shaderHandle;
			FreeMemory();
			m_Data.clear();
			m_NameToTextureAndSlot.clear();

			for (auto uniform : shader->GetUniforms()) {
				//m_Data[uniform->GetName()] = Utils::AllocateMemoryByShaderDataType(uniform->GetType());
				m_Data[uniform->GetName()].Allocate(Utils::GetAllocatedMemoryByShaderDataType(uniform->GetType()));
				m_Data[uniform->GetName()].ZeroInitialize();
			}

			//deal with texture
			for (auto uniform : shader->GetUniforms()) {
				if (uniform->GetType() == ShaderDataType::Sampler2D) {
					//  todo Should I store the address of the shared pointer or the raw pointer's ?
					Ref<Texture2D> defaultTexture = Renderer::GetDefaultBlackQuadTexture();
					Handle<Texture2D> defaultTextureHandle = ResourceManager<Texture2D>::Add(defaultTexture);
					m_Data[uniform->GetName()].Write(defaultTexture.get());
					m_NameToTextureAndSlot[uniform->GetName()] = std::pair(defaultTextureHandle, m_NameToTextureAndSlot.size());
				}
			}


			//ShaderLibrary::LinkMaterial(shader->GetHandle(), this->GetHandle());
			
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
