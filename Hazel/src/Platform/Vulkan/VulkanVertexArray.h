#pragma once


#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Resource/Resource.h"
#include <vulkan/vulkan.h>

namespace Hazel {

	class VertexBuffer;
	class IndexBuffer;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;

	class VulkanVertexArray
	{
	public:
		VulkanVertexArray() = default;
		~VulkanVertexArray() = default;
						
		void										AddVertexBuffer		(const Handle<VertexBuffer>& vertexBuffer);		
		void										SetIndexBuffer		(const Handle<IndexBuffer>& indexBuffer) { m_IndexBuffer = indexBuffer; }		
		const std::vector<Handle<VertexBuffer>>&	GetVertexBuffers	() const { return m_VertexBuffers; }
		const Handle<IndexBuffer>&					GetIndexBuffer		() const { return m_IndexBuffer; }
				
	private:
		
		std::vector<Handle<VertexBuffer>>	m_VertexBuffers;
		Handle<IndexBuffer>					m_IndexBuffer;		

	};

} 
