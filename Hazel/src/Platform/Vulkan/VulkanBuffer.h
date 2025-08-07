

#include "Hazel/Renderer/Buffer.h"


#include "vulkan/vulkan.h"

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


	private:

		

		VkBuffer	GetRawBuffer() const { return m_Buffer; }
		
		static void Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);


	private:


		VkBuffer m_Buffer;


		friend class VulkanVertexBuffer;

	};


	class VulkanVertexBuffer : public VertexBuffer {

	public:

		VulkanVertexBuffer(float* vertices, uint32_t size);
		VulkanVertexBuffer(void* vertices, uint32_t size);
		VulkanVertexBuffer(uint32_t size);
		~VulkanVertexBuffer();

		VkVertexInputBindingDescription						GetBindingDescription();
		std::vector<VkVertexInputAttributeDescription>		GetAttributeDescriptions();		
		inline VkBuffer										GetRawBuffer() { return m_Buffer; }

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, const uint32_t size) override;



	private:

		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;

	};


	class VulkanIndexBuffer : public IndexBuffer {


	public:

		VulkanIndexBuffer();

		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		VulkanIndexBuffer(void* indices, uint32_t count);
		~VulkanIndexBuffer();

		virtual void Bind() const override {}
		virtual void Unbind() const override {}

		virtual uint32_t GetCount() const override { return m_Count; }
		virtual VkBuffer GetRawBuffer() const { return m_Buffer; }

	private:
		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
		uint32_t m_Count;

	};

	class VulkanUniformBuffer : public UniformBuffer {

	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		~VulkanUniformBuffer();		
		
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		inline VkBuffer					GetRawBuffer()				const { return m_Buffer; }

		inline void*					GetMappedData()				const { return m_MappedData; };
		inline VkDescriptorBufferInfo	GetDescriptorBufferInfo()	const { return m_DescriptorBufferInfo; }

	private:

		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
		VkDescriptorBufferInfo m_DescriptorBufferInfo;
		void* m_MappedData;

	};



}