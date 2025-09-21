#include "hzpch.h"

#include "Platform/Vulkan/VulkanVertexArray.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Hazel{

	void VulkanVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		m_VertexBuffers.push_back(vertexBuffer);
		m_RawBuffers.push_back(std::dynamic_pointer_cast<VulkanVertexBuffer>(vertexBuffer)->GetRawBuffer());
	}

}