#pragma once

#include "Hazel/Renderer/Pipeline.h"

#include "vulkan/vulkan.h"


namespace Hazel {

	class VulkanShader;

	namespace Utils {

		VkPrimitiveTopology GetVulkanPrimitiveTopology(PrimitiveTopology type);		
	}


	class VulkanPipeline : public Pipeline{




	public:

		VulkanPipeline(const PipelineSpecification& specification);

		

		inline VkPipeline			GetRawPipeline()	{ return m_GraphicsPipeline; }

		inline VkPipelineLayout		GetPipelineLayout() { return m_Layout; }

	private:

		Ref<VulkanShader> m_Shader;
		
		VkPipeline m_GraphicsPipeline;

		VkPipelineLayout m_Layout;


	};



}


