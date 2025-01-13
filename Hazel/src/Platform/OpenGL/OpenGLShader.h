#pragma once


#include "Hazel/Renderer/Shader.h"
 



namespace Hazel {

	class OpenGLShader : public Shader {
	public:
		OpenGLShader() = default;
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual const std::string GetName() const override;

		virtual const void SetInt(const std::string& name, const int val) override;
		virtual const void SetFloat(const std::string& name, const float val) override;
		virtual const void SetFloat2(const std::string& name, const glm::vec2& val) override;
		virtual const void SetFloat3(const std::string& name, const glm::vec3& val) override;
		virtual const void SetFloat4(const std::string& name, const glm::vec4& val) override;
		virtual const void SetMat3(const std::string& name, const glm::mat3& val) override;
		virtual const void SetMat4(const std::string& name, const glm::mat4& val) override;
		//Array
		virtual const void SetIntArray(const std::string& name, const int* val, const uint32_t count) override;


		virtual bool Reload() override;

		void UploadUniformInt(const std::string& name, const int& val);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vec2);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vec3);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vec4);
		void UploadUniformFloat(const std::string& name, const float& val);
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

		void UploadUniformIntArray(const std::string& name, const int* val, const uint32_t count);

		virtual const GLint GetRendererID() override;


		virtual void Submit(std::unordered_map<std::string, void*> data) override;
		



	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> Preprocess(std::string& src);
		bool Compile(const std::unordered_map<GLenum, std::string> shaderSourcescode);
	private:
		uint32_t m_RendererID;
	};

}