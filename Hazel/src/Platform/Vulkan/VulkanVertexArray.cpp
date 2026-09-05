#include "hzpch.h"

#include "Platform/Vulkan/VulkanVertexArray.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Hazel{

	void VulkanVertexArray::AddVertexBuffer(const Handle<VertexBuffer>& vertexBuffer)
	{
		m_VertexBuffers.push_back(vertexBuffer);
		HZ_CORE_ASSERT(false, "VulkanVertexArray needs a VulkanVertexBuffer proxy or a handle-backed lookup path");
	}

	
}
