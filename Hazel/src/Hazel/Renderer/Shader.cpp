#include "hzpch.h"
#include "Hazel/Renderer/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel {
	
	std::unordered_map<std::string, std::shared_ptr<Hazel::Shader>> Hazel::ShaderLibrary::m_Shaders;

	namespace Utils {
		std::string GetShaderName(std::string filepath) {
			int lastSlash = filepath.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash;
			int lastDot = filepath.find_last_of('.');
			int count = lastDot == std::string::npos ? filepath.size() - lastSlash - 1 : lastDot - lastSlash - 1;
			return filepath.substr(lastSlash + 1, count);
		}
	}

	std::vector<Ref<ShaderUniform>> Shader::GetUniforms()
	{
		return m_Uniforms;
	}

	Ref<ShaderUniform> Shader::GetUniform(std::string name)
	{

		for (auto uniform : m_Uniforms) {
			if (uniform->GetName() == name)
				return uniform;
		}
		return nullptr;
	}

	std::string Shader::GetPath()
	{
		return m_Path;
	}

	Ref<Shader> Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(vertexSrc, fragmentSrc);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(filepath);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	void Shader::AddAssocitaedMaterial(Material* material)
	{
		this->m_AssociatedMaterials.insert(material);
	}

	void Shader::DeleteAssocitaedMaterial(Material* material)
	{
		if (m_AssociatedMaterials.find(material) != m_AssociatedMaterials.end()) {
			m_AssociatedMaterials.erase(material);
		}
	}

	void ShaderLibrary::Add(const Ref<Shader> shader)
	{
		auto& name = shader->GetName();
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader> shader)
	{
		if (Exists(name))
		{
			HZ_CORE_INFO("This Shader already exist{}", name);
			return;
		}
		m_Shaders[name] = shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& path)
	{
		
		std::string name = Utils::GetShaderName(path);
		if (Exists(name))
			return m_Shaders[name];
		
		auto shader = Shader::Create(path);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& path)
	{
		if (Exists(name))
			return m_Shaders[name];
		auto shader = Shader::Create(path);
		Add(name, shader);
		return shader;
	}

	void ShaderLibrary::Reload(const std::string& name)
	{
		m_Shaders[name]->Reload();
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		if(Exists(name))
			return m_Shaders[name];
		HZ_CORE_ASSERT(false, "This Shader not exists!");
	}
	bool ShaderLibrary::Exists(const std::string& name) {
		return m_Shaders.find(name) != m_Shaders.end();
	}
}