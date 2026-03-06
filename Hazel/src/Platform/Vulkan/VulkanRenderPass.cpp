#include "hzpch.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

#include "Hazel/Renderer/Renderer.h"

#include "vulkan/vulkan.h"

#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanPipeline.h"


namespace Hazel {

	namespace Utils {

		ByteKey GetRawRenderPassByteKey(const RenderPassSpecification& spec) {

			auto attachmentSpec = spec.attachmentSpecs;

			ByteKey byteKey;
			for (auto& spec : attachmentSpec) {
				byteKey.AddBytes(spec.format);
				byteKey.AddBytes(spec.isClearColor);
				byteKey.AddBytes(spec.clearValue.color);
			}

			return byteKey;

		}		

	}


	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification) : RenderPass(specification) {
				

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		
		m_RenderPass = VulkanRenderPass::GetRawRenderPass(m_Specification);

		if (m_Specification.pipeline != nullptr) {

			VulkanRendererAPI::RegisterRenderPipeline(m_Specification.pipeline, m_RenderPass);
			DescriptorSetManagerSpecification descriptorSetManagerSpec;
			descriptorSetManagerSpec.usage = DescriptorSetManagerUsage::RenderPass;
			descriptorSetManagerSpec.shader = m_Specification.pipeline->GetSpecification().shader;

			m_DescriptorSetManager = DescriptorSetManager::Create(descriptorSetManagerSpec);
		}
				
	}

	VkRenderPass VulkanRenderPass::GetRawRenderPass(const RenderPassSpecification& spec)
	{
		return VulkanRendererAPI::GetRawRenderPass(spec);
	}

	VkRenderPass VulkanRenderPass::CreateRawRenderPass(const RenderPassSpecification& spec)
	{

		Ref<VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		VkRenderPass vkRenderPass = nullptr;

		std::vector<VkAttachmentDescription> attachmentDescriptions;
		std::vector<VkAttachmentReference> colorAttachmentRefs;

		VkAttachmentDescription depthAttachment{};
		VkAttachmentReference depthAttachmentRef{};
		bool hasDepth = false;

		uint32_t attachmentIndex = 0;

		for (auto attachment : spec.attachmentSpecs) {

			if (Utils::IsDepthFormat(attachment.format)) {

				// force the depth attachment to be the last one :)

				depthAttachment.format = Utils::GetVulkanFormatFromTextureFormat(attachment.format);
				depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				depthAttachment.loadOp = attachment.isClearColor ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				depthAttachmentRef.attachment = spec.attachmentSpecs.size() - 1;
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				hasDepth = true;

				continue;
			}


			VkAttachmentDescription& attachemnt = attachmentDescriptions.emplace_back();
			attachemnt.format = Utils::GetVulkanFormatFromTextureFormat(attachment.format);
			if (spec.pipeline != nullptr) {
				attachemnt.samples = VkSampleCountFlagBits(spec.pipeline->GetSpecification().multiSampleCount);
			}
			else {
				attachemnt.samples = VkSampleCountFlagBits(1);
			}
			
			attachemnt.loadOp = attachment.isClearColor ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachemnt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachemnt.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachemnt.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference& colorAttachmentRef = colorAttachmentRefs.emplace_back();
			colorAttachmentRef.attachment = attachmentIndex;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachmentIndex++;
		}


		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colorAttachmentRefs.size();
		subpass.pColorAttachments = colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		if (hasDepth) {
			attachmentDescriptions.emplace_back(depthAttachment);
		}

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentDescriptions.size();
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		HZ_CORE_ASSERT(vkCreateRenderPass(device->GetRawDevice(), &renderPassInfo, nullptr, &vkRenderPass) == VK_SUCCESS, "Failed to create render pass for framebuffer");

		return vkRenderPass;
		
	}

	void VulkanRenderPass::SetPipeline(Ref<Pipeline> pipeline)
	{
		m_Specification.pipeline = pipeline;

		VulkanRendererAPI::GetRenderPipeline(pipeline->GetSpecification(), this->m_RenderPass);

	}

	void VulkanRenderPass::SetPipelineState(const PipelineSpecification& spec)
	{
		m_Specification.pipeline = VulkanRendererAPI::GetRenderPipeline(spec, this->m_RenderPass);
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