#pragma once

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/VertexArray.h"


namespace Hazel {


	class OpenGLVertexArray : public VertexArray{
	public:

		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override ;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
		virtual uint32_t GetRendererID() const override;
		
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const override;

		static OpenGLVertexArray* Create();
	private:
		uint32_t m_RendererID;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
		uint32_t m_VertexAtrributeAmount;
	};
}