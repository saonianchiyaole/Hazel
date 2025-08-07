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

namespace Hazel {
	
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

	}

	void VulkanRendererAPI::SetDepthTest(bool value)
	{
	}

	void VulkanRendererAPI::SetDepthMask(bool value)
	{
	}

	void VulkanRendererAPI::BeginFrame() {

	}

	void VulkanRendererAPI::EndFrame()
	{

	}

	void VulkanRendererAPI::DrawFrame()
	{

	}

	void VulkanRendererAPI::BeginRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass, uint32_t imageIndex) {

		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		Ref<VulkanRenderPass> vulkanRenderPass = std::static_pointer_cast<VulkanRenderPass>(renderPass);


		Ref<VulkanSwapchain> swapchain = VulkanContext::GetCurrentContext()->GetSwapchain();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vulkanRenderPass->GetRawRenderPass();
		renderPassInfo.framebuffer = swapchain->GetFramebuffers()[imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchain->GetDetails().swapChainExtent;


		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(vulkanCommandBuffer->GetRawCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkPipeline pipeline = vulkanRenderPass->GetPipeline()->GetRawPipeline();

		vkCmdBindPipeline(vulkanCommandBuffer->GetRawCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);


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

	}


	void VulkanRendererAPI::EndRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass) {

		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);

		vkCmdEndRenderPass(vulkanCommandBuffer->GetRawCommandBuffer());

	}

	void VulkanRendererAPI::BindVertexArray(Ref<CommandBuffer> commandBuffer, Ref<VertexArray> vertexArray)
	{

		/*for(int i = 0; i < vertexArray->GetVertexBuffers().size(); i++)
		{
			Ref<VulkanVertexBuffer> vertexBuffer = vertexArray->GetVertexBuffers()[i];
			BindVertexBuffer(commandBuffer, vertexBuffer);
		}

		BindVertexBuffer(commandBuffer, vertexArray->GetVertexBuffers());*/

	}

	void VulkanRendererAPI::BindIndexBuffer(Ref<CommandBuffer> commandBuffer, Ref<IndexBuffer> indexBuffer)
	{
		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		Ref<VulkanIndexBuffer> vulkanIndexBuffer = std::static_pointer_cast<VulkanIndexBuffer>(indexBuffer);
		vkCmdBindIndexBuffer(vulkanCommandBuffer->GetRawCommandBuffer(), vulkanIndexBuffer->GetRawBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void VulkanRendererAPI::DrawIndexed(Ref<CommandBuffer> commandBuffer, uint32_t count)
	{
		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		vkCmdDrawIndexed(vulkanCommandBuffer->GetRawCommandBuffer(), count, 1, 0, 0, 0);
	}

	void VulkanRendererAPI::BindVertexBuffer(Ref<CommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer)
	{
		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		Ref<VulkanVertexBuffer> vulkanVertexBuffer = std::static_pointer_cast<VulkanVertexBuffer>(vertexBuffer);

		VkBuffer vertexBuffers[] = { vulkanVertexBuffer->GetRawBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(vulkanCommandBuffer->GetRawCommandBuffer(), 0, 1, vertexBuffers, offsets);
	}

	void VulkanRendererAPI::SubmitMaterial(Ref<CommandBuffer> commandBuffer, Ref<Material> material)
	{
		static uint32_t currentFrame = 0;

		Ref<VulkanCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		Ref<VulkanMaterial> vulkanMaterial = std::static_pointer_cast<VulkanMaterial>(material);
		

		vkCmdBindDescriptorSets(vulkanCommandBuffer->GetRawCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, /* todo remove this*/vulkanMaterial->pipelineLayout, 0, vulkanMaterial->GetDescriptorSets()[currentFrame].size(), vulkanMaterial->GetDescriptorSets()[currentFrame].data(), 0, nullptr);
		
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
}



