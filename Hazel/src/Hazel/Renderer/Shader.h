#pragma once

#include <string>

#include "glm/glm.hpp"
#include "glad/glad.h"

#include "Hazel/Renderer/ShaderUniform.h"

namespace Hazel {

	namespace Utils {
		std::string GetShaderName(std::string path);
	}

	enum class ShaderType {
		Null = 0,
		VertexShader, FragmentShader,
		ComputeShader, VertAndFragShader
	};



	class Shader {
	public:
		Shader() = default;
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual const std::string GetName() const = 0;
			

		virtual bool Reload() = 0;

		virtual const void SetInt(const std::string& name, const int val) = 0;
		virtual const void SetFloat(const std::string& name, const float val) = 0;
		virtual const void SetFloat2(const std::string& name, const glm::vec2& val) = 0;
		virtual const void SetFloat3(const std::string& name, const glm::vec3& val) = 0;
		virtual const void SetFloat4(const std::string& name, const glm::vec4& val) = 0;
		virtual const void SetMat3(const std::string& name, const glm::mat3& val) = 0;
		virtual const void SetMat4(const std::string& name, const glm::mat4& val) = 0;

		virtual const void SetIntArray(const std::string& name, const int* val, const uint32_t count) = 0;

		virtual const GLint GetRendererID() = 0;



		virtual void Submit(std::unordered_map<std::string, void*> data) = 0;
		std::vector<Ref<ShaderUniform>> GetUniforms();
		std::string GetPath();
		//Ref<ShaderUniform> GetUniform(std::string name);

		static Ref<Shader> Create(const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& filepath);
	protected:
		std::string m_Name;
		std::string m_Path;
		std::vector<Ref<ShaderUniform>> m_Uniforms;
		ShaderType m_Type;

	};


	class ShaderLibrary {
	public:
		static void Add(const Ref<Shader> shader);
		static void Add(const std::string& name, const Ref<Shader> shader);
		static Ref<Shader> Load(const std::string& path);
		static Ref<Shader> Load(const std::string& name, const std::string& path);
		static void Reload(const std::string& name);


		static bool Exists(const std::string& name);
		static Ref<Shader> Get(const std::string& name);
	private:

		friend class Editor;
		static std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};

}