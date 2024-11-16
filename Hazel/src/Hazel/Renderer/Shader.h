#pragma once

#include <string>

#include "glm/glm.hpp"
#include "glad/glad.h"

namespace Hazel {

	class Shader {
	public:
		Shader() = default;
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual const std::string GetName() const = 0;


		virtual const void SetInt(const std::string& name, const int val) = 0;
		virtual const void SetFloat(const std::string& name, const float val) = 0;
		virtual const void SetFloat2(const std::string& name, const glm::vec2& val) = 0;
		virtual const void SetFloat3(const std::string& name, const glm::vec3& val) = 0;
		virtual const void SetFloat4(const std::string& name, const glm::vec4& val) = 0;
		virtual const void SetMat3(const std::string& name, const glm::mat3& val) = 0;
		virtual const void SetMat4(const std::string& name, const glm::mat4& val) = 0;

		virtual const void SetIntArray(const std::string& name, const int* val, const uint32_t count) = 0;


		virtual const GLint GetRendererID() = 0;


		static Ref<Shader> Create(const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& filepath);
	private:
	};


	class ShaderLibrary {
	public:
		static void Add(const Ref<Shader> shader);
		static void Add(const std::string& name, const Ref<Shader> shader);
		static void Load(const std::string& path);
		static void Load(const std::string& name, const std::string& path);

		static bool Exists(const std::string& name);
		static Ref<Shader> Get(const std::string& name);
	private:
		static std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};

}