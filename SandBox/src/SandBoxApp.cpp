#include "Hazel.h"

#include "Hazel/Event/ApplicationEvent.h"

#include "imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Hazel/Renderer/OrthographicCameraController.h"

#include "Hazel/Core/EntryPoint.h"

//#include "Hazel/Core/EntryPoint.h"
#include "Sandbox2D.h"
#include <optional>
#include <fstream>


#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/DescriptorSetManager.h"
#include "Platform/Vulkan/VulkanMaterial.h"


#include "Hazel/Renderer/Pipeline.h"
#include "Hazel/Renderer/Shader.h"
#include "vulkan/vulkan.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan_win32.h>


std::vector<float> vertices = {
	0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 1.0f, 1.0f, 0.0f
};

std::vector<uint32_t> indices = {
	0, 1, 2, 3, 0, 2
};

struct UniformBufferObject {

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

};


using namespace Hazel;

class ExampleLayer : public Hazel::ImGuiLayer {
public:
	ExampleLayer()
		:ImGuiLayer() {


		
		m_UBO.model = glm::mat4(1.0f);
		m_UBO.view = m_Camera.GetViewMatrix();
		m_UBO.projection = glm::mat4(1.0f);

		RenderCommand::Init();		


		
		// TODO: Move this to Vulkan Renderer
		VulkanContext::GetCurrentContext()->GetDevice()->CreateCommandPool();
		VulkanContext::GetCurrentContext()->GetDevice()->CreateCommandBuffers();
		

		m_VertexBuffer = MakeRef<VulkanVertexBuffer>(vertices.data(), vertices.size() * sizeof(float));

		Hazel::BufferLayout layout = std::vector<Hazel::BufferElement>{
			Hazel::BufferElement{ Hazel::ShaderDataType::Float2, "Position"},
			Hazel::BufferElement{ Hazel::ShaderDataType::Float3, "Color"}
		};

		m_VertexBuffer->SetLayout(layout);	

		m_IndexBuffer = MakeRef<VulkanIndexBuffer>(indices.data(),	indices.size());

		m_Shader = Shader::Create("assets/Shaders/vulkanShader.glsl");

		m_Material = MakeRef<VulkanMaterial>(m_Shader);
		m_Material->Set<UniformBufferObject>("UniformBufferObject", m_UBO);

		
		Ref<VulkanShader> vulkanshader = std::dynamic_pointer_cast<VulkanShader>(m_Shader);
		

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.shader = m_Shader;
		pipelineSpecification.bufferLayout = layout;		

		m_Pipeline = Pipeline::Create(pipelineSpecification);

		RenderPassSpecification renderPassSpec = {
			nullptr, m_Pipeline
		};

		m_RenderPass = MakeRef<VulkanRenderPass>(renderPassSpec);

		Ref<VulkanPipeline> vulkanPipeline = std::dynamic_pointer_cast<VulkanPipeline>(m_Pipeline);
		m_Material->pipelineLayout = vulkanPipeline->GetPipelineLayout();


	}

	void OnAttach() override {

	}

	void OnDetach() override {

	}

	void OnUpdate(Hazel::Timestep ts) override {

		

		//Hazel::VulkanContext::GetDevice()->DrawFrame();

		static int currentFrame = 0;

		Ref<Hazel::VulkanDevice> device = VulkanContext::GetCurrentContext()->GetDevice();
		Ref<Hazel::VulkanSwapchain> swapchain = VulkanContext::GetCurrentContext()->GetSwapchain();

		std::vector<VkSemaphore> imageAvailableSemaphores = swapchain->GetImageAvailableSemaphores();
		std::vector<VkSemaphore> renderFinishedSemaphores = swapchain->GetRenderFinishedSemaphores();
		std::vector<VkFence> InFlightFences = swapchain->GetInFlightFences();
		Ref<VulkanCommandBuffer> commandBuffer = device->GetCommandBuffers()[currentFrame];


		vkWaitForFences(device->GetRawDevice(), 1, &InFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(device->GetRawDevice(), 1, &InFlightFences[currentFrame]);

		uint32_t imageIndex;

		VkResult result = vkAcquireNextImageKHR(device->GetRawDevice(), swapchain->GetRawSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		
			swapchain->Recreate(Application::GetInstance().GetWindow().GetWidth(), Application::GetInstance().GetWindow().GetHeight(), Application::GetInstance().GetWindow().IsVSync());
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		vkResetCommandBuffer(device->GetCommandBuffers()[currentFrame]->GetRawCommandBuffer(), 0);

		commandBuffer->Begin();

		RenderCommand::BeginRenderPass(commandBuffer, m_RenderPass, imageIndex);
		RenderCommand::BindVertexBuffer(commandBuffer, m_VertexBuffer);
		RenderCommand::BindIndexBuffer(commandBuffer, m_IndexBuffer);		

		Ref<VulkanShader> vulkanshader = std::dynamic_pointer_cast<VulkanShader>(m_Shader);
		RenderCommand::SubmitMaterial(commandBuffer, m_Material);
		RenderCommand::DrawIndexed(commandBuffer, indices.size());
		RenderCommand::EndRenderPass(commandBuffer, m_RenderPass);

		commandBuffer->End();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer->GetRawCommandBuffer();

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(device->GetGraphicQueue(), 1, &submitInfo, InFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapchain->GetRawSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		//presentInfo.pResults = nullptr;


		vkQueuePresentKHR(device->GetPresentQueue(), &presentInfo);

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;




		//glfw
	}

	virtual void OnImGuiRender() override {


	}

	void OnEvent(Hazel::Event& event) override {


		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Hazel::KeyPressedEvent& e) {
		//HZ_INFO("{0}", e);
		return false;
	}


private:


	//Resources

	Ref<VertexBuffer> m_VertexBuffer;
	Ref<RenderPass> m_RenderPass;
	Ref<Pipeline> m_Pipeline;	
	Ref<Shader> m_Shader;

	Ref<VulkanMaterial> m_Material;

	std::vector<VkBuffer> uniformBuffer;
	std::vector<VkDeviceMemory> uniformBufferMemory;
	std::vector<void*> uniformBuffersMapped;

	Ref<IndexBuffer> m_IndexBuffer;

	Camera m_Camera;
	UniformBufferObject m_UBO;

};


class Sandbox :public Hazel::Application {
public:
	Sandbox() {
		//PushOverlay(new ExampleLayer());
		PushLayer(new ExampleLayer());
	}

	Sandbox(std::string windowName) : Application(windowName) {
		PushLayer(new ExampleLayer());
	}

	~Sandbox() {

	}
};

Hazel::Application* Hazel::CreateApplication() {

	return new Sandbox("Vulkan Test");

}



