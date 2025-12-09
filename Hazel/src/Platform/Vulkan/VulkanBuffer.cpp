#include "hzpch.h"


#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Hazel {



	namespace Utils {


		uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {

			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type!");

		}


		VkFormat GetVulkanFormatFormShaderDataType(ShaderDataType type) {

			switch (type)
			{
			case ShaderDataType::Float:     return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2:    return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Int:       return VK_FORMAT_R32_SINT;
			case ShaderDataType::Int2:      return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:      return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:      return VK_FORMAT_R32G32B32A32_SINT;
			}
			HZ_CORE_ASSERT(false, "Invalid Shader data type!");
			return VK_FORMAT_UNDEFINED;
		}




		VkVertexInputBindingDescription GetBindingDescription(BufferLayout layout) {
			VkVertexInputBindingDescription bindingDescription{};

			bindingDescription.binding = 0;
			bindingDescription.stride = layout.GetStride();
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(BufferLayout layout) {

			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
			attributeDescriptions.resize(layout.GetElements().size());

			for (uint32_t i = 0; i < attributeDescriptions.size(); i++) {

				// TODO : this should fit every case of binding
				attributeDescriptions[i].binding = 0;
				attributeDescriptions[i].location = i;
				attributeDescriptions[i].format = Utils::GetVulkanFormatFormShaderDataType(layout.GetElements()[i].Type);
				attributeDescriptions[i].offset = layout.GetElements()[i].Offset;

			}

			return attributeDescriptions;

		}


		VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

			VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
			VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create vertex buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate vertex buffer memory!");
			}

			vkBindBufferMemory(device, buffer, bufferMemory, 0);

			return buffer;
		}

		void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size, Ref<VulkanDevice> device)
		{

			if (device == nullptr) {
				device = VulkanContext::GetCurrentContext()->GetDevice();				
			}

			HZ_CORE_ASSERT(device != nullptr, "No valid vulkan device!");

			Ref<VulkanCommandBuffer> commandBuffer = MakeRef<VulkanCommandBuffer>();

			commandBuffer->Begin();

			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer->GetRawCommandBuffer(), src, dst, 1, &copyRegion);

			commandBuffer->End();			
			commandBuffer->Submit();
		}

	}

	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size) {


		m_Size = size;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();

		// staging buffer

		VkDeviceSize bufferSize = size;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		Utils::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, vertices, size);
		vkUnmapMemory(device, stagingBufferMemory);

		// vertex buffer


		Utils::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_Memory);
		Utils::CopyBuffer(stagingBuffer, m_Buffer, size);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

	}

	VulkanVertexBuffer::VulkanVertexBuffer(void* vertices, uint32_t size)
	{

		m_Size = size;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();

		// staging buffer

		VkDeviceSize bufferSize = size;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		Utils::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
			
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &vertices);
		memcpy(data, vertices, size);
		vkUnmapMemory(device, stagingBufferMemory);

		// vertex buffer


		Utils::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_Memory);
		Utils::CopyBuffer(stagingBuffer, m_Buffer, size);
		
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

	}

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{

		m_Size = size;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		Utils::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_Buffer, m_Memory);
		
		vkMapMemory(device, m_Memory, 0, size, 0, &m_MappedData);

	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		vkDestroyBuffer(device, m_Buffer, nullptr);
		vkFreeMemory(device, m_Memory, nullptr);
	}

	VkVertexInputBindingDescription VulkanVertexBuffer::GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};

		bindingDescription.binding = 0;
		bindingDescription.stride = m_Layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}


	std::vector<VkVertexInputAttributeDescription> VulkanVertexBuffer::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.resize(m_Layout.GetElements().size());

		for (int i = 0; i < attributeDescriptions.size(); i++) {

			// TODO : this should fit every case of binding
			attributeDescriptions[i].binding = 0;
			attributeDescriptions[i].location = i;
			attributeDescriptions[i].location = Utils::GetVulkanFormatFormShaderDataType(m_Layout.GetElements()[i].Type);
			attributeDescriptions[i].offset = m_Layout.GetElements()[i].Offset;
		}

		return attributeDescriptions;
	}

	void VulkanVertexBuffer::Bind() const
	{
	}

	void VulkanVertexBuffer::Unbind() const
	{
	}

	void VulkanVertexBuffer::SetData(const void* data, const uint32_t size)
	{
		
		HZ_CORE_ASSERT(m_Size >= size, "Allocated data's size is out of range!");
		memcpy(m_MappedData, data, size);

	}
	

	//---------------------------------------------Vulkan Index Buffer---------------------------------------------


	VulkanIndexBuffer::VulkanIndexBuffer() {



	}


	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		vkDestroyBuffer(device, m_Buffer, nullptr);
		vkFreeMemory(device, m_Memory, nullptr);
	}

	void VulkanIndexBuffer::SetData(uint32_t* indices, uint32_t count)
	{


	}

	VulkanIndexBuffer::VulkanIndexBuffer(void* indices, uint32_t count)
	{
		m_Count = count;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();


		// staging buffer

		VkDeviceSize bufferSize = count * sizeof(uint32_t);
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		void* voidIndicePtr = indices;

		Utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &voidIndicePtr);
		memcpy(data, indices, bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		// vertex buffer

		Utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_Memory);
		Utils::CopyBuffer(stagingBuffer, m_Buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	
	
	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
	{
		m_Count = count;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();
	

		// staging buffer

		VkDeviceSize bufferSize = count * sizeof(uint32_t);
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		
		Utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices, bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		// vertex buffer

		Utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_Memory);
		Utils::CopyBuffer(stagingBuffer, m_Buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

	}

	// ------------------------------------------- Uniform Buffer -------------------------------------------




	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
	{
		m_Size = size;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		VkDeviceSize bufferSize = size;

		Utils::CreateBuffer(bufferSize, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			m_Buffer, m_Memory);

		vkMapMemory(device, m_Memory, 0, bufferSize, 0, &m_MappedData);

		m_DescriptorBufferInfo.buffer = m_Buffer;
		m_DescriptorBufferInfo.offset = 0;
		m_DescriptorBufferInfo.range = size;
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		vkFreeMemory(device, m_Memory, nullptr);
		vkDestroyBuffer(device, m_Buffer, nullptr);
	}

	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		HZ_CORE_ASSERT(m_Size >= size, "Allocated data's size is out of range!");
		memcpy(m_MappedData, data, size);
	}

}