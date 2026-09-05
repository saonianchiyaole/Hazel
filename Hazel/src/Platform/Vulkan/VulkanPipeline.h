#pragma once

#include "Hazel/Renderer/Pipeline.h"

#include "Hazel/Core/ByteKey.h"

#include "vulkan/vulkan.h"

namespace Hazel {

	class VulkanShader;

	namespace Utils {

		VkPrimitiveTopology GetVulkanPrimitiveTopology(PrimitiveTopology type);		

		ByteKey GetRenderPipelineByteKey(const PipelineInfo& spec, VkRenderPass renderPass);


	}


	class VulkanPipeline{

	public:

		VulkanPipeline() = default;

		
		bool						Init(const PipelineInfo& specification, VkRenderPass renderPass);

		inline VkPipeline			GetRawPipeline()	{ return m_GraphicsPipeline; }

		inline VkPipelineLayout		GetPipelineLayout() { return m_Layout; }

		inline const PipelineInfo&	GetPipelineInfo() { return m_Info; }

		static Ref<VulkanPipeline>	CreateVulkanPipeline(const PipelineInfo& spec, const VkRenderPass renderPass);		

	private:
				

		PipelineInfo m_Info;

		VkPipeline m_GraphicsPipeline;

		VkPipelineLayout m_Layout;


	};



}


