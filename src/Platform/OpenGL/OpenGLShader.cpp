#include "hzpch.h"
#include "Platform/OpenGL/OpenGLShader.h"


#include <glm/gtc/type_ptr.hpp>


#include <fstream>

namespace Hazel {


	GLenum ShaderTypeFromString(const std::string& type) {
		if (type == "vertex") {
			return GL_VERTEX_SHADER;
		}
		else if (type == "fragment" || type == "pixel") {
			return GL_FRAGMENT_SHADER;
		}
		HZ_CORE_ASSERT(false, "Invaild shader type specified!");
	}

	OpenGLShader::OpenGLShader(const std::string& filepath) {
		
		std::string shaderSource = ReadFile(filepath);
		Compile(Preprocess(shaderSource));

		int lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash;
		int lastDot = filepath.find_last_of('.');
		int count = lastDot == std::string::npos ? filepath.size() - lastSlash - 1 : lastDot - lastSlash - 1;
		m_Name = filepath.substr(lastSlash + 1, count);
	}


	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc) {
		std::unordered_map<GLenum, std::string> shaderSourceCode;
		shaderSourceCode[GL_VERTEX_SHADER] = vertexSrc;
		shaderSourceCode[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(shaderSourceCode);
		
	}
	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
		std::unordered_map<GLenum, std::string> shaderSourceCode;
		shaderSourceCode[GL_VERTEX_SHADER] = vertexSrc;
		shaderSourceCode[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(shaderSourceCode);
		m_Name = name;

	}
	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in, std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else {
			HZ_CORE_ERROR("Could not open file '{0}'", filepath);
		}
		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::Preprocess(std::string& src)
	{
		std::unordered_map<GLenum, std::string> result;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = src.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = src.find_first_of("\r\n", pos);
			HZ_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = src.substr(begin, eol - begin);
			HZ_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel", "Invaild shader type specified!");

			size_t nextLinePos = src.find_first_not_of("\r\n", eol);
			pos = src.find(typeToken, nextLinePos);
			result[ShaderTypeFromString(type)] = 
				src.substr(nextLinePos, 
				pos - (nextLinePos == std::string::npos ? src.size() - 1 : nextLinePos));
		}

		return result;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string> shaderSourcescode)
	{
		
		GLint program = glCreateProgram();
		std::vector<GLint> shaderIDs(shaderSourcescode.size());
		for (auto& kv : shaderSourcescode) {
			GLenum type = kv.first;

			// Create an empty vertex shader handle
			GLuint shader = glCreateShader(type);

			const GLchar* source = (const GLchar*)kv.second.c_str();
			glShaderSource(shader, 1, &source, 0);

			// Compile the vertex shader
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// We don't need the shader anymore.
				glDeleteShader(shader);

				// Use the infoLog as you see fit.

				HZ_CORE_ERROR("{0}", infoLog.data());
				HZ_CORE_ASSERT(false, "Shader compilation failure!");

				// In this simple program, we'll just leave
				return;
			}

			glAttachShader(program, shader);
			shaderIDs.push_back(shader);
		}
		
		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			for (auto& shaderID : shaderIDs) {
				glDeleteShader(shaderID);
			}

			// Use the infoLog as you see fit.

			// In this simple program, we'll just leave
			return;
		}

		m_RendererID = program;
		// Always detach shaders after a successful link.
		for(auto& shaderID : shaderIDs)
		{
			glDetachShader(program, shaderID);
			glDetachShader(program, shaderID);
		}
	}



	void OpenGLShader::Bind() const {
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const {
		glUseProgram(0);
	}

	const std::string OpenGLShader::GetName() const
	{
		return m_Name;
	}

	const void OpenGLShader::SetInt(const std::string& name, const int val)
	{
		UploadUniformInt(name, val);
	}

	const void OpenGLShader::SetFloat(const std::string& name, const float val)
	{
		UploadUniformFloat(name, val);
	}
	const void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& val)
	{
		UploadUniformFloat2(name, val);
	}
	const void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& val)
	{
		UploadUniformFloat3(name, val);
	}
	const void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& val)
	{
		UploadUniformFloat4(name, val);
	}
	const void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& val)
	{
		UploadUniformMat3(name, val);
	}
	const void OpenGLShader::SetMat4(const std::string& name, const  glm::mat4& val)
	{
		UploadUniformMat4(name, val);
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vec4)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, vec4.x, vec4.y, vec4.z, vec4.w);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vec3)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, vec3.x, vec3.y, vec3.z);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vec2)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, vec2.x, vec2.y);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const float& val)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, val);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, const int& val)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, val);
	}

	


}