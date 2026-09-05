#include "hzpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include "glad/glad.h"
namespace Hazel {

	/*  -------------------------------Vertex Buffer--------------------------------*/
	
	OpenGLVertexBuffer::~OpenGLVertexBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(size_t size, std::vector<uint8_t> data)
	{
		m_Size = size;
		if (!data.empty() && data.size() < size)
			data.resize(size);
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, data.empty() ? nullptr : data.data(), GL_STATIC_DRAW);
	}

	void OpenGLVertexBuffer::Bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, const uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}


	/* ----------------------------Index Buffer-----------------------------------*/

	OpenGLIndexBuffer::OpenGLIndexBuffer(size_t size, std::vector<uint8_t> data)
	{
		m_Size = size;
		m_Count = static_cast<uint32_t>(size / sizeof(uint32_t));
		if (!data.empty() && data.size() < size)
			data.resize(size);
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data.empty() ? nullptr : data.data(), GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{		
		glDeleteBuffers(1, &m_RendererID);
	}	

	void OpenGLIndexBuffer::Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}
	void OpenGLIndexBuffer::Unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void OpenGLIndexBuffer::SetData(uint32_t* data, uint32_t count) {

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint8_t), data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	/* ----------------------------Uniform Buffer-----------------------------------*/

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding){

		m_Size = size;

		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::SetData(const void* data, size_t size, size_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}


}
