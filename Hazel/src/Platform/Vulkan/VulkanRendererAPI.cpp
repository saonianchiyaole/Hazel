#include "hzpch.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanVertexArray.h"
#include "Platform/Vulkan/VulkanMaterial.h"

#include <imgui.h>



namespace Hazel {
	

	VulkanRendererAPI::RendererData* VulkanRendererAPI::s_Data = nullptr;

	void Hazel::VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
	}

	void VulkanRendererAPI::Clear()
	{
	}

	void VulkanRendererAPI::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{

	}

	void VulkanRendererAPI::Init()
	{		
		VulkanContext::GetCurrentContext()->GetDevice()->CreateCommandPool();			

		// todo shouldn't be here
		Ref<VulkanSwapchain> swapchain = std::static_pointer_cast<VulkanSwapchain>(Application::GetInstance().GetWindow().GetSwapchain());
		Ref<VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		swapchain->CreateCommandBuffers();

		s_Data = new RendererData;

		// ------------------------------------ Device -----------------------------------------------
		s_Data->device = device;


		// ------------------------------------  Create Descriptor Pool ------------------------------------  

		
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10 * 3; // frames in flight should partially determine this
		poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);;
		poolInfo.pPoolSizes = poolSizes;


		s_Data->descriptorPools.resize(MAX_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {			
			HZ_CORE_ASSERT(
				vkCreateDescriptorPool(device->GetRawDevice(), &poolInfo, nullptr, &s_Data->descriptorPools[i]) == VK_SUCCESS,
				"Failed to create descriptor pool!"
			);
		}
				
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
	{

	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
	}
	
	

	void VulkanRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{

	}

	void VulkanRendererAPI::SetLineWidth(float width)
	{
		//vkCmdSetLineWidth(vulkanCommandBuffer->GetRawCommandBuffer(), width);
	}

	void VulkanRendererAPI::SetDepthTest(bool value)
	{
	}

	void VulkanRendererAPI::SetDepthMask(bool value)
	{
	}

	void VulkanRendererAPI::BeginFrame() {

		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();		
		vkResetDescriptorPool(s_Data->device->GetRawDevice(), s_Data->descriptorPools[frameIndex], 0);
	}



	void VulkanRendererAPI::EndFrame()
	{

	}

	void VulkanRendererAPI::DrawFrame()
	{

	}

	void VulkanRendererAPI::BeginRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass) {


		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);		
		Ref<VulkanSwapchain> swapchain = VulkanContext::GetCurrentContext()->GetSwapchain();
		Ref<VulkanFramebuffer> framebuffer = std::static_pointer_cast<VulkanFramebuffer>(renderPass->GetPipeline()->GetTargetFramebuffer());

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = framebuffer->GetRawRenderPass();
		renderPassInfo.framebuffer = framebuffer->GetRawFramebuffer();

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchain->GetDetails().swapChainExtent;


		std::array<VkClearValue, 2> clearColor;
		clearColor[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearColor[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = clearColor.size();
		renderPassInfo.pClearValues = clearColor.data();
	
		vkCmdBeginRenderPass(vulkanCommandBuffer->GetRawCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		Ref<VulkanPipeline> pipeline = std::static_pointer_cast<VulkanPipeline>(renderPass->GetPipeline());
		VkPipeline rawPipeline = pipeline->GetRawPipeline();

		vkCmdBindPipeline(vulkanCommandBuffer->GetRawCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, rawPipeline);


		// dynamic part
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->GetDetails().swapChainExtent.width);
		viewport.height = static_cast<float>(swapchain->GetDetails().swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(vulkanCommandBuffer->GetRawCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain->GetDetails().swapChainExtent;
		vkCmdSetScissor(vulkanCommandBuffer->GetRawCommandBuffer(), 0, 1, &scissor);

		vkCmdSetLineWidth(vulkanCommandBuffer->GetRawCommandBuffer(), pipeline->GetLineWidth());
	}


	void VulkanRendererAPI::EndRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass) {

		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		
			
		vkCmdEndRenderPass(vulkanCommandBuffer->GetRawCommandBuffer());

		Ref<Framebuffer> framebuffer = renderPass->GetPipeline()->GetTargetFramebuffer();

		std::vector<Ref<Texture2D>> textures = framebuffer->GetColorAttachments();

		for (Ref<Texture2D> texture : textures) {
			Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(texture);
			vulkanTexture->SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

	}

	void VulkanRendererAPI::DrawIndexed(Ref<CommandBuffer> commandBuffer, const Ref<VertexArray>& vertexArray)
	{
		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);

		Ref<VulkanVertexArray> vulkanVertexArray = std::static_pointer_cast<VulkanVertexArray>(vertexArray);

		VkCommandBuffer rawCommandBuffer = vulkanCommandBuffer->GetRawCommandBuffer();

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(rawCommandBuffer, 0, vertexArray->GetVertexBuffers().size(), 
			vulkanVertexArray->GetRawBuffers().data(), offsets);

		vkCmdBindIndexBuffer(rawCommandBuffer, 
			std::static_pointer_cast<VulkanIndexBuffer>(vulkanVertexArray->GetIndexBuffer())->GetRawBuffer(),
			0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(rawCommandBuffer, vertexArray->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);

	}

	
	void VulkanRendererAPI::DrawIndexed(Ref<CommandBuffer> commandBuffer, const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);

		Ref<VulkanVertexArray> vulkanVertexArray = std::static_pointer_cast<VulkanVertexArray>(vertexArray);

		VkCommandBuffer rawCommandBuffer = vulkanCommandBuffer->GetRawCommandBuffer();

		vkCmdBindVertexBuffers(rawCommandBuffer, 0, vertexArray->GetVertexBuffers().size(),
			vulkanVertexArray->GetRawBuffers().data(), 0);

		vkCmdBindIndexBuffer(rawCommandBuffer,
			std::static_pointer_cast<VulkanIndexBuffer>(vulkanVertexArray->GetIndexBuffer())->GetRawBuffer(),
			0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(rawCommandBuffer, vertexArray->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRendererAPI::SubmitMaterial(Ref<CommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
	{
		Ref<VulkanPipeline> vulkanPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		Ref<VulkanMaterial> vulkanMaterial = std::static_pointer_cast<VulkanMaterial>(material);
		material->Submit();

		vkCmdBindDescriptorSets(vulkanCommandBuffer->GetRawCommandBuffer(), 
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			vulkanPipeline->GetPipelineLayout(),
			0, vulkanMaterial->GetDescriptorSets()[0].size(),
			vulkanMaterial->GetDescriptorSets()[0].data(),
			0, nullptr);

	}

	VkDescriptorSet VulkanRendererAPI::AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();
		VkDescriptorSet descriptorSet;	
		allocInfo.descriptorPool = s_Data->descriptorPools[frameIndex];
		vkAllocateDescriptorSets(s_Data->device->GetRawDevice(), &allocInfo, &descriptorSet);
		return descriptorSet;
	}	
	
}



