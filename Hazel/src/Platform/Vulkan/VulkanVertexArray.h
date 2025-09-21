#pragma once


#include "Hazel/Renderer/VertexArray.h"



#include <vulkan/vulkan.h>

namespace Hazel {

	class VertexBuffer;
	class IndexBuffer;

	class VulkanVertexArray : public VertexArray
	{
	public:
		VulkanVertexArray() = default;
		~VulkanVertexArray() = default;
			
		virtual void Bind() const override {}
		virtual void Unbind() const override {}
	
		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;

		virtual uint32_t GetRendererID() const override { return 0; };

		std::vector<VkBuffer> GetRawBuffers() { return m_RawBuffers; }


	private:

		std::vector<VkBuffer> m_RawBuffers;

	};

} 