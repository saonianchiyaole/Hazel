#pragma once


#include "Hazel/Renderer/VertexArray.h"




namespace Hazel {

	class VertexBuffer;
	class IndexBuffer;

	class VulkanVertexArray : public VertexArray
	{
	public:
		VulkanVertexArray();
		virtual ~VulkanVertexArray();
			
		virtual void Bind() const override;
		virtual void Unbind() const override;

		//Set
		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override { m_VertexBuffers.push_back(vertexBuffer); }
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override { m_IndexBuffer = indexBuffer; };
		
		virtual const std::vector<Ref<VertexBuffer>>&	GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const Ref<IndexBuffer>&					GetIndexBuffer() const override { return m_IndexBuffer; };


		virtual uint32_t GetRendererID() {};

	private:

		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

	};

} 