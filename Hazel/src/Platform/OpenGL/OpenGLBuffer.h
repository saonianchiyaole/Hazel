#pragma once

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {

	class OpenGLVertexBuffer : public VertexBuffer {
	public:
		virtual ~OpenGLVertexBuffer();
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		OpenGLVertexBuffer(void* vertices, uint32_t size);
		OpenGLVertexBuffer(uint32_t size);

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, const uint32_t size) override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; };
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer {
	public:
		OpenGLIndexBuffer(){}
		virtual ~OpenGLIndexBuffer();
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		OpenGLIndexBuffer(void* indices, uint32_t count);

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};


	class OpenGLUniformBuffer : public UniformBuffer {
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset) override;
	private:
		uint32_t m_RendererID;
	};

}