#pragma once

#include "Hazel/Renderer/RenderPass.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/DescriptorSetManager.h"


namespace Hazel {


	class VulkanRenderPass : public RenderPass {

	public:

		VulkanRenderPass(const RenderPassSpecification& specification);

		inline		VkRenderPass			GetRawRenderPass()	const	{ return m_RenderPass; }
		
		inline		Ref<VulkanPipeline>		GetPipeline()				{ return std::dynamic_pointer_cast<VulkanPipeline>(m_Specification.pipeline); }

		operator	VkRenderPass()								const	{ return m_RenderPass; }


		
		virtual bool SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) override;
		virtual bool SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) override;
		virtual bool SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index = 0) override;

		virtual void Submit() override;

		const std::unordered_map<uint32_t, VkDescriptorSet>& GetDescriptorSets();

	private:

		VkRenderPass m_RenderPass;			 		

		Scope<DescriptorSetManager> m_DescriptorSetManager;



	};

}
