#pragma once

#include "Hazel/Renderer/RenderPass.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/DescriptorSetManager.h"


namespace Hazel {


	namespace Utils {


		ByteKey GetRawRenderPassByteKey(const RenderPassSpecification& spec);

	}

	class VulkanRenderPass : public RenderPass {

	public:

		VulkanRenderPass(const RenderPassSpecification& specification);

		inline		VkRenderPass			GetRawRenderPass()	const	{ return m_RenderPass; }

		
		virtual bool	SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) override;
		virtual bool	SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) override;
		virtual bool	SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index = 0) override;		

		virtual void	Submit() override;

		virtual void	SetPipeline(Ref<Pipeline> pipeline) override;
		virtual void	SetPipelineState(const PipelineSpecification& spec) override;

		const std::unordered_map<uint32_t, VkDescriptorSet>& GetDescriptorSets();
	


		static VkRenderPass GetRawRenderPass(const RenderPassSpecification& spec);

		static VkRenderPass CreateRawRenderPass(const RenderPassSpecification& spec);


	private:

		VkRenderPass m_RenderPass;			
		

		Scope<DescriptorSetManager> m_DescriptorSetManager;



	};

}
