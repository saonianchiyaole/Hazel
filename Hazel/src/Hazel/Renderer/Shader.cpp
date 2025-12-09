#include "hzpch.h"
#include "Hazel/Renderer/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Renderer/Material.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Hazel {
	
	std::unordered_map<std::string, std::shared_ptr<Shader>> ShaderLibrary::s_Shaders;

	std::unordered_map<AssetHandle, std::vector<AssetHandle>> ShaderLibrary::s_AssociatedMaterials;

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
			return MakeRef<OpenGLShader>(filepath);
		case RendererAPI::API::Vulkan:
			return MakeRef<VulkanShader>(filepath);
		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}


	void ShaderLibrary::Add(const Ref<Shader> shader)
	{
		auto& name = shader->GetName();

		if (Exists(name))
		{
			HZ_CORE_INFO("This Shader already exist{}", name);
			return;
		}
		s_Shaders[name] = shader;

		AssetManager::AddAsset(shader);
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader> shader)
	{
		if (Exists(name))
		{
			HZ_CORE_INFO("This Shader already exist{}", name);
			return;
		}
		s_Shaders[name] = shader;
		AssetManager::AddAsset(shader);
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& path)
	{
		
		std::string name = Utils::GetShaderName(path);
		if (Exists(name))
			return s_Shaders[name];
		
		auto shader = Shader::Create(path);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& path)
	{
		if (Exists(name))
			return s_Shaders[name];
		auto shader = Shader::Create(path);
		Add(name, shader);
		return shader;
	}

	void ShaderLibrary::Reload(const std::string& name)
	{

		Ref<Shader> shader = s_Shaders[name];
		shader->Reload();
		if (s_AssociatedMaterials.find(shader->GetHandle()) != s_AssociatedMaterials.end()) {
			
			for(auto& materialHandle : s_AssociatedMaterials[shader->GetHandle()]) {
				auto material = AssetManager::GetAsset(materialHandle)->As<Material>();
				if (material) {
					material->ReloadShader();
				}
			}

		}
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		if(Exists(name))
			return s_Shaders[name];
		HZ_CORE_ASSERT(false, "This Shader not exists!");
	}
	bool ShaderLibrary::LinkMaterial(const AssetHandle shader, const AssetHandle material)
	{
		if (s_AssociatedMaterials.find(shader) != s_AssociatedMaterials.end()) {
			s_AssociatedMaterials[shader].push_back(material);
			return true;
		}

		s_AssociatedMaterials[shader] = std::vector<AssetHandle>();
		s_AssociatedMaterials[shader].push_back(material);

	}

	bool ShaderLibrary::UnlinkMaterial(const AssetHandle material)
	{
		for(auto it : s_AssociatedMaterials) {
			auto& vec = it.second;
			auto found = std::find(vec.begin(), vec.end(), material);
			if (found != vec.end()) {
				vec.erase(found);
				return true;
			}
		}
		return false;
	}

	bool ShaderLibrary::UnlinkShader(const AssetHandle shader)
	{
		if(s_AssociatedMaterials.find(shader) == s_AssociatedMaterials.end())
			return false;
		s_AssociatedMaterials.erase(shader);
		return true;
	}

	bool ShaderLibrary::Exists(const std::string& name) {
		return s_Shaders.find(name) != s_Shaders.end();
	}
}