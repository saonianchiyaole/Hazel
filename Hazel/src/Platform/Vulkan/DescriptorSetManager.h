#pragma once


#include <vulkan/vulkan.h>	


namespace Hazel {

	class VulkanUniformBuffer;
	class CommandBuffer;
	class VulkanShader;

	struct DescriptorSetSpecification {
		
		VkDescriptorSetLayoutBinding binding;
		VkDescriptorType descriptorType;
		VkShaderStageFlags stageFlags;
		
		uint32_t descriptorCount = 1;
	
	};

	struct DescriptorSpecification {

		uint32_t binding;		

	};


	class DescriptorSetManager
	{

	public:


		void Init();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();
		void AllocateDescriptorSet(Ref<VulkanShader> shader, uint32_t frameIndex);
		void WriteDescriptorBufferMemory();
		void Bind(Ref<CommandBuffer> commandBuffer, VkPipelineLayout pipelineLayout, uint32_t imageIndex);		

		inline VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

	private:
		
		VkDescriptorPool m_DescriptorPool;				

	};



}