#pragma once

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {

	class OpenGLVertexBuffer : public VertexBuffer {
	public:
		virtual ~OpenGLVertexBuffer();		
		OpenGLVertexBuffer(size_t size, std::vector<uint8_t> data = {});

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, const uint32_t size) override;	

	private:
		uint32_t m_RendererID;	
	};

	class OpenGLIndexBuffer : public IndexBuffer {
	public:
		OpenGLIndexBuffer(size_t size, std::vector<uint8_t> data = {});
		virtual ~OpenGLIndexBuffer();
		
		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(uint32_t* indices, uint32_t count);

		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};


	class OpenGLUniformBuffer : public UniformBuffer {
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const void* data, size_t size, size_t offset = 0) override;
	private:
		uint32_t m_RendererID;
	};

}
