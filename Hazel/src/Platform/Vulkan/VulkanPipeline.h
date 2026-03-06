#pragma once

#include "Hazel/Renderer/Pipeline.h"

#include "Hazel/Core/ByteKey.h"

#include "vulkan/vulkan.h"

namespace Hazel {

	class VulkanShader;

	namespace Utils {

		VkPrimitiveTopology GetVulkanPrimitiveTopology(PrimitiveTopology type);		

		ByteKey GetRenderPipelineByteKey(const PipelineSpecification& spec, VkRenderPass renderPass);


	}


	class VulkanPipeline : public Pipeline{

	public:

		VulkanPipeline(const PipelineSpecification& specification);

		
		bool						Init(VkRenderPass renderPass);
		inline VkPipeline			GetRawPipeline()	{ return m_GraphicsPipeline; }

		inline VkPipelineLayout		GetPipelineLayout() { return m_Layout; }


		static Ref<VulkanPipeline>	CreateVulkanPipeline(const PipelineSpecification& spec, const VkRenderPass renderPass);		

	private:

		Ref<VulkanShader> m_Shader;
		
		VkPipeline m_GraphicsPipeline;

		VkPipelineLayout m_Layout;


	};



}


