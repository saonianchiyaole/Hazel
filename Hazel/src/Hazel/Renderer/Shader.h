#pragma once

#include <string>

#include "glm/glm.hpp"
#include "glad/glad.h"

#include "Hazel/Renderer/ShaderUniform.h"
#include "Hazel/Core/Buffer.h"
#include "Hazel/Resource/Resource.h"

namespace Hazel {

	namespace Utils {
		std::string GetShaderName(std::string path);
	}

	enum class ShaderType {
		None = 0,
		VertexShader, FragmentShader,
		ComputeShader, Normal
	};

	enum class DescriptorType {
		Sampler, UniformBuffer, StorageBuffer, Sampler2D, SamplerCube, Sampler3D
	};


	struct ShaderReflectionData {
		std::string name;
		DescriptorType type;
		uint32_t binding;
		uint32_t descriptorSet;
		uint32_t size;
		uint32_t offset = 0;
		uint32_t arraySize = 0;
		ShaderType stage;

		uint32_t dimension = 1;
	};


	struct ShaderSnapshot {
		std::string name;
		std::string path;
		std::unordered_map<ShaderType, std::string> shaderCodes;		
		std::unordered_map<ShaderType, std::vector<uint32_t>> spirvBinarys;		
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderReflectionData>> relectionData;
		std::unordered_map<std::string, ShaderReflectionData> relectionDataByName;
	};


	class Material;

	class Shader : Resource{

	friend class Material;

	public:


		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		Shader(const std::string& filepath);
		Shader() = default;
		virtual ~Shader() = default;

		virtual void Bind() const {}
		virtual void Unbind() const {}
		virtual const std::string GetName() const{ return m_Name; };
			
		Ref<const ShaderSnapshot> CreateSnapShot();

		virtual bool Reload();

		virtual const void SetInt		(const std::string& name, const int val)		{}
		virtual const void SetFloat		(const std::string& name, const float val)		{}
		virtual const void SetFloat2	(const std::string& name, const glm::vec2& val) {}
		virtual const void SetFloat3	(const std::string& name, const glm::vec3& val) {}
		virtual const void SetFloat4	(const std::string& name, const glm::vec4& val) {}
		virtual const void SetMat3		(const std::string& name, const glm::mat3& val)	{}
		virtual const void SetMat4		(const std::string& name, const glm::mat4& val)	{}

		virtual const void SetIntArray(const std::string& name, const int* val, const uint32_t count) {}

		virtual ShaderReflectionData* GetReflectionDataByName(const std::string& name) {
			if (m_RelectionDataByName.find(name) != m_RelectionDataByName.end()) {
				return &m_RelectionDataByName.at(name);
			}
			return nullptr;
		}

		virtual const GLint GetRendererID() { return 0; }

		ResourceType GetResourceType() {
			return ResourceType::Shader;
		}

		virtual void Submit(std::unordered_map<std::string, Buffer>& data) {}
		std::vector<Ref<ShaderUniform>> GetUniforms();
		Ref<ShaderUniform>				GetUniform(std::string);
		std::string						GetPath();
		const std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderReflectionData>>& GetReflectionData() const { return m_RelectionData; }
		const std::unordered_map<ShaderType, std::vector<uint32_t>>& GetSPIRVBinarys() const { return m_SPIRVBinarys; }
		//Ref<ShaderUniform> GetUniform(std::string name);

		static Ref<Shader> Create(const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& filepath);

	protected:		

		std::string m_Name;
		std::string m_Path;
		std::unordered_map<ShaderType, std::string> m_ShaderCodes;
		std::unordered_map<ShaderType, std::vector<uint32_t>> m_SPIRVBinarys;
		std::vector<Ref<ShaderUniform>> m_Uniforms;
		ShaderType m_Type;

		std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderReflectionData>> m_RelectionData;
		std::unordered_map<std::string, ShaderReflectionData> m_RelectionDataByName;
		
	};


	class ShaderLibrary {
	public:
		static void Add(const Ref<Shader> shader);
		static void Add(const std::string& name, const Ref<Shader> shader);
		static Handle<Shader> Load(const std::string& path);
		static Handle<Shader> Load(const std::string& name, const std::string& path);
		static void Reload(const std::string& name);


		static bool Exists(const std::string& name);
		static Handle<Shader> Get(const std::string& name);

		static bool LinkMaterial(const Handle<Shader> shader, const Handle<Material> material);
		static bool UnlinkMaterial(const Handle<Material> material);
		static bool UnlinkShader(const Handle<Shader> shader);
	private:

		friend class Editor;
		static std::unordered_map<std::string, Handle<Shader>> s_ShaderHandles;

		static std::unordered_map<Handle<Shader>, std::vector<Handle<Material>>> s_AssociatedMaterials;

	};

}
