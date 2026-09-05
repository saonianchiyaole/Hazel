#pragma once

#include "vulkan/vulkan.h"
#include "Hazel/Renderer/Buffer.h"

namespace Hazel {


	class VulkanDevice;

	namespace Utils {


		uint32_t											FindMemoryType						(VkPhysicalDevice physicalDevice, 
																								 uint32_t typeFilter, 
																								 VkMemoryPropertyFlags properties);

		VkFormat											GetVulkanFormatFormShaderDataType	(ShaderDataType type);
		VkVertexInputBindingDescription						GetBindingDescription				(BufferLayout layout);
		std::vector<VkVertexInputAttributeDescription>		GetAttributeDescriptions			(BufferLayout layout);

		VkBuffer											CreateBuffer						(VkDeviceSize size,
																								 VkBufferUsageFlags usage,
																								 VkMemoryPropertyFlags properties,
																								 VkBuffer& buffer,
																								 VkDeviceMemory& bufferMemory);

		void												CopyBuffer							(VkBuffer src, 
																								 VkBuffer dst,
																								 VkDeviceSize size,
																								 Ref<VulkanDevice> device = nullptr);
	}



	class VulkanBuffer {


	protected:

		VulkanBuffer() = default;

		VulkanBuffer(size_t size) : m_Size(size) {};

		VkBuffer GetRawBuffer() const { return m_Buffer; }
				
	protected:


		VkBuffer m_Buffer;
		size_t m_Size = 0;

	};


	class VulkanVertexBuffer : public VulkanBuffer{

	public:
		
		VulkanVertexBuffer(size_t size, std::vector<uint8_t> data);
		~VulkanVertexBuffer();

		VkVertexInputBindingDescription						GetBindingDescription();
		std::vector<VkVertexInputAttributeDescription>		GetAttributeDescriptions();		
		inline VkBuffer										GetRawBuffer() { return m_Buffer; }

		void Bind() const;
		void Unbind() const;

		void SetData(const void* data, const uint32_t size);
		void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
		const BufferLayout& GetLayout() const { return m_Layout; }



	private:
		
		VkDeviceMemory m_Memory;
		void* m_MappedData;		
		BufferLayout m_Layout;

	};


	class VulkanIndexBuffer : VulkanBuffer {
	public:

		VulkanIndexBuffer();

		VulkanIndexBuffer(size_t size, std::vector<uint8_t> data = {});
				
		~VulkanIndexBuffer();
		
		uint32_t GetCount() const { return m_Count; }
		VkBuffer GetRawBuffer() const { return m_Buffer; }

		void SetData(uint32_t* indices, uint32_t count);

	private:
		
		VkDeviceMemory m_Memory;
		uint32_t m_Count;

	};

	class VulkanUniformBuffer : VulkanBuffer{

	public:
		VulkanUniformBuffer(size_t size, std::vector<uint8_t> data);
		~VulkanUniformBuffer();		
		
		void									SetData(const void* data, size_t size, size_t offset = 0);
		inline	VkBuffer						GetRawBuffer()				const	{ return m_Buffer; }

		inline void*							GetMappedData()				const	{ return m_MappedData; };
		inline			VkDescriptorBufferInfo	GetDescriptorBufferInfo()			{ return m_DescriptorBufferInfo; }

	private:
		
		VkDeviceMemory m_Memory;
		VkDescriptorBufferInfo m_DescriptorBufferInfo;
		void* m_MappedData;		

	};



}
