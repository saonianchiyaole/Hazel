#pragma once

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {

	class VertexArray {
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		//Set
		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) { m_VertexBuffers.push_back(vertexBuffer); }
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) { m_IndexBuffer = indexBuffer; }
		//Get
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; };
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; };


		virtual uint32_t GetRendererID() const = 0;

		static Ref<VertexArray> Create();


	protected:

		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

	};


}