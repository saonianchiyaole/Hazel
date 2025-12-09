#include "hzpch.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

#include "Hazel/Renderer/Renderer.h"

#include "vulkan/vulkan.h"

#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanShader.h"


namespace Hazel {


	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification) : RenderPass(specification) {
				
		m_Specification.pipeline = specification.pipeline;

		m_Specification.targetFramebuffer = m_Specification.pipeline->GetTargetFramebuffer();
		

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		

		DescriptorSetManagerSpecification descriptorSetManagerSpec;
		descriptorSetManagerSpec.usage = DescriptorSetManagerUsage::RenderPass;
		descriptorSetManagerSpec.shader = m_Specification.pipeline->GetSpecification().shader;

		m_DescriptorSetManager = DescriptorSetManager::Create(descriptorSetManagerSpec);		

	}

	bool VulkanRenderPass::SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index) {
		return m_DescriptorSetManager->SetData(name, uniformBufferSet, index);
	}

	bool VulkanRenderPass::SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index)
	{
		return m_DescriptorSetManager->SetData(name, uniformBuffer, index);
	}


	bool VulkanRenderPass::SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index)
	{
		return m_DescriptorSetManager->SetData(name, texture, index);
	}

	void VulkanRenderPass::Submit()
	{
		m_DescriptorSetManager->Submit();
	}

	const std::unordered_map<uint32_t, VkDescriptorSet>& VulkanRenderPass::GetDescriptorSets()
	{
		return m_DescriptorSetManager->GetDescriptorSets();
	}

}