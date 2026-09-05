#include "hzpch.h"
#include "Hazel/Renderer/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Resource/ResourceManager.h"

#include "Hazel/Renderer/Material.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

#include <fstream>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Hazel {
	
	std::unordered_map<std::string, Handle<Shader>> ShaderLibrary::s_ShaderHandles;
	std::unordered_map<Handle<Shader>, std::vector<Handle<Material>>> ShaderLibrary::s_AssociatedMaterials;

	namespace Utils {
		std::string GetShaderName(std::string filepath) {
			int lastSlash = filepath.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash;
			int lastDot = filepath.find_last_of('.');
			int count = lastDot == std::string::npos ? filepath.size() - lastSlash - 1 : lastDot - lastSlash - 1;
			return filepath.substr(lastSlash + 1, count);
		}
	}

	namespace
	{
		std::string ReadShaderFile(const std::string& filepath)
		{
			std::string result;
			std::ifstream in(filepath, std::ios::in | std::ios::binary);
			if (in) {
				in.seekg(0, std::ios::end);
				result.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(result.data(), result.size());
				in.close();
			}
			else {
				HZ_CORE_ERROR("Could not open shader file '{0}'", filepath);
			}

			return result.substr(0, result.find_first_of('\0'));
		}

		ShaderType GetShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return ShaderType::VertexShader;
			if (type == "fragment" || type == "pixel")
				return ShaderType::FragmentShader;
			if (type == "compute")
				return ShaderType::ComputeShader;

			HZ_CORE_ASSERT(false, "Invalid shader type specified!");
			return ShaderType::None;
		}

		shaderc_shader_kind GetShaderCKindFromShaderType(ShaderType type)
		{
			switch (type) {
			case ShaderType::VertexShader:
				return shaderc_vertex_shader;
			case ShaderType::FragmentShader:
				return shaderc_fragment_shader;
			case ShaderType::ComputeShader:
				return shaderc_compute_shader;
			case ShaderType::None:
			default:
				HZ_CORE_ASSERT(false, "Invalid shader type!");
				return shaderc_glsl_infer_from_source;
			}
		}

		std::unordered_map<ShaderType, std::string> PreprocessShaderSource(const std::string& source)
		{
			std::unordered_map<ShaderType, std::string> result;

			const char* typeToken = "#type";
			const size_t typeTokenLength = strlen(typeToken);
			size_t pos = source.find(typeToken, 0);

			while (pos != std::string::npos) {
				size_t eol = source.find_first_of("\r\n", pos);
				HZ_CORE_ASSERT(eol != std::string::npos, "Shader syntax error!");

				size_t begin = pos + typeTokenLength + 1;
				std::string type = source.substr(begin, eol - begin);
				HZ_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel" || type == "compute", "Invalid shader type specified!");

				size_t nextLinePos = source.find_first_not_of("\r\n", eol);
				pos = source.find(typeToken, nextLinePos);

				result[GetShaderTypeFromString(type)] = source.substr(
					nextLinePos,
					pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
			}

			return result;
		}

		ShaderDataType GetShaderDataTypeFromReflectionData(const ShaderReflectionData& reflectionData)
		{
			switch (reflectionData.type) {
			case DescriptorType::Sampler2D:
				return ShaderDataType::Sampler2D;
			case DescriptorType::SamplerCube:
				return ShaderDataType::SamplerCube;
			default:
				return ShaderDataType::None;
			}
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
		default:
			return MakeRef<Shader>(vertexSrc, fragmentSrc);
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
		default:
			return MakeRef<Shader>(name, vertexSrc, fragmentSrc);
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
		default:
			return MakeRef<Shader>(filepath);
		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		m_Name = "Shader";
		m_Type = ShaderType::Normal;
		m_ShaderCodes[ShaderType::VertexShader] = vertexSrc;
		m_ShaderCodes[ShaderType::FragmentShader] = fragmentSrc;
		Reload();
	}

	Shader::Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		m_Name = name;
		m_Type = ShaderType::Normal;
		m_ShaderCodes[ShaderType::VertexShader] = vertexSrc;
		m_ShaderCodes[ShaderType::FragmentShader] = fragmentSrc;
		Reload();
	}

	Shader::Shader(const std::string& filepath)
	{
		m_Path = filepath;
		m_Name = Utils::GetShaderName(filepath);
		Reload();
	}


	Ref<const ShaderSnapshot> Shader::CreateSnapShot()
	{

		Ref<ShaderSnapshot> snapShot = MakeRef<ShaderSnapshot>();

		snapShot->name = m_Name;
		snapShot->path = m_Path;
		snapShot->shaderCodes = m_ShaderCodes;
		snapShot->spirvBinarys = m_SPIRVBinarys;
		snapShot->relectionData = m_RelectionData;
		snapShot->relectionDataByName = m_RelectionDataByName;

		return snapShot;
	}

	bool Shader::Reload()
	{
		if (!m_Path.empty()) {
			std::string shaderSource = ReadShaderFile(m_Path);
			m_ShaderCodes = PreprocessShaderSource(shaderSource);
		}

		m_SPIRVBinarys.clear();
		m_RelectionData.clear();
		m_RelectionDataByName.clear();
		m_Uniforms.clear();

		static shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		options.SetWarningsAsErrors();
		options.SetOptimizationLevel(shaderc_optimization_level_zero);

		for (const auto& [shaderType, sourceCode] : m_ShaderCodes) {
			const std::string compileName = m_Path.empty() ? m_Name : m_Path;
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
				sourceCode,
				GetShaderCKindFromShaderType(shaderType),
				compileName.c_str(),
				options);

			if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
				HZ_CORE_WARN("{} while compiling shader: {}", module.GetErrorMessage(), compileName);
				return false;
			}

			m_SPIRVBinarys[shaderType] = std::vector<uint32_t>(module.begin(), module.end());
		}

		for (const auto& [shaderType, spirvBinary] : m_SPIRVBinarys) {
			spirv_cross::Compiler compiler(spirvBinary);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			for (const auto& resource : resources.uniform_buffers) {
				auto activeBuffers = compiler.get_active_buffer_ranges(resource.id);
				if (activeBuffers.empty())
					continue;

				const auto& baseType = compiler.get_type(resource.base_type_id);
				const auto& type = compiler.get_type(resource.type_id);

				ShaderReflectionData reflectionData;
				reflectionData.name = resource.name;
				reflectionData.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				reflectionData.descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				reflectionData.size = static_cast<uint32_t>(compiler.get_declared_struct_size(baseType));
				reflectionData.stage = shaderType;
				reflectionData.type = DescriptorType::UniformBuffer;
				reflectionData.arraySize = type.array.empty() ? 1 : type.array[0];
				reflectionData.dimension = 1;

				m_RelectionData[reflectionData.descriptorSet][reflectionData.binding] = reflectionData;
				m_RelectionDataByName[reflectionData.name] = reflectionData;
			}

			for (const auto& resource : resources.sampled_images) {
				const auto& baseType = compiler.get_type(resource.base_type_id);
				const auto& type = compiler.get_type(resource.type_id);

				ShaderReflectionData reflectionData;
				reflectionData.name = resource.name;
				reflectionData.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				reflectionData.descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				reflectionData.size = 8;
				reflectionData.stage = shaderType;
				reflectionData.dimension = baseType.image.dim + 1;
				reflectionData.arraySize = type.array.empty() ? 1 : type.array[0];

				switch (reflectionData.dimension) {
				case 3:
					reflectionData.type = DescriptorType::Sampler3D;
					break;
				case 4:
					reflectionData.type = DescriptorType::SamplerCube;
					break;
				case 1:
				case 2:
				default:
					reflectionData.type = DescriptorType::Sampler2D;
					break;
				}

				m_RelectionData[reflectionData.descriptorSet][reflectionData.binding] = reflectionData;
				m_RelectionDataByName[reflectionData.name] = reflectionData;

				ShaderDataType uniformType = GetShaderDataTypeFromReflectionData(reflectionData);
				if (uniformType != ShaderDataType::None) {
					m_Uniforms.push_back(MakeRef<ShaderUniform>(reflectionData.name, uniformType));
				}
			}
		}

		return true;
	}


	void ShaderLibrary::Add(const Ref<Shader> shader)
	{
		auto& name = shader->GetName();

		if (Exists(name))
		{
			HZ_CORE_INFO("This Shader already exist{}", name);
			return;
		}
		Handle<Shader> handle = ResourceManager<Shader>::Add(shader);

		s_ShaderHandles[name] = handle;

	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader> shader)
	{
		if (Exists(name))
		{
			HZ_CORE_INFO("This Shader already exist{}", name);
			return;
		}

		Handle<Shader> handle = ResourceManager<Shader>::Add(shader);

		s_ShaderHandles[name] = handle;
		
	}

	Handle<Shader> ShaderLibrary::Load(const std::string& path)
	{
		
		std::string name = Utils::GetShaderName(path);
		if (Exists(name))
			return s_ShaderHandles[name];
		
		auto shader = Shader::Create(path);
		Add(shader);
		return s_ShaderHandles[name];
	}

	Handle<Shader> ShaderLibrary::Load(const std::string& name, const std::string& path)
	{
		if (Exists(name))
			return s_ShaderHandles[name];
		auto shader = Shader::Create(path);
		Add(name, shader);
		return s_ShaderHandles[name];
	}

	void ShaderLibrary::Reload(const std::string& name)
	{

		Handle<Shader> shaderHandle = s_ShaderHandles[name];
		auto shader = ResourceManager<Shader>::Get(shaderHandle);
		shader->Reload();
		if (s_AssociatedMaterials.find(shaderHandle) != s_AssociatedMaterials.end()) {
			
			for(auto& materialHandle : s_AssociatedMaterials[shaderHandle]) {
				auto material = ResourceManager<Material>::Get(materialHandle);
				if (material) {
					material->ReloadShader();
				}
			}

		}
	}

	Handle<Shader> ShaderLibrary::Get(const std::string& name)
	{
		if(Exists(name))
			return s_ShaderHandles[name];
		HZ_CORE_ASSERT(false, "This Shader not exists!");
		return Handle<Shader>();
	}

	bool ShaderLibrary::LinkMaterial(const Handle<Shader> shader, const Handle<Material> material)
	{
		if (s_AssociatedMaterials.find(shader) != s_AssociatedMaterials.end()) {
			s_AssociatedMaterials[shader].push_back(material);
			return true;
		}

		s_AssociatedMaterials[shader] = std::vector<Handle<Material>>();
		s_AssociatedMaterials[shader].push_back(material);
		return true;
	}

	bool ShaderLibrary::UnlinkMaterial(const Handle<Material> material)
	{
		for(auto& it : s_AssociatedMaterials) {
			auto& vec = it.second;
			auto found = std::find(vec.begin(), vec.end(), material);
			if (found != vec.end()) {
				vec.erase(found);
				return true;
			}
		}
		return false;
	}

	bool ShaderLibrary::UnlinkShader(const Handle<Shader> shader)
	{
		if(s_AssociatedMaterials.find(shader) == s_AssociatedMaterials.end())
			return false;
		s_AssociatedMaterials.erase(shader);
		return true;
	}

	bool ShaderLibrary::Exists(const std::string& name) {
		return s_ShaderHandles.find(name) != s_ShaderHandles.end();
	}	

}
