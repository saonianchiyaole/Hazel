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
	0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	-0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
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
		m_UBO.projection = m_Camera.GetProjectionMatrix();

		RenderCommand::Init();		

		m_Texture2D = TextureLibrary::Load("assets/Checkboard.png");
				
		m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));			

		Hazel::BufferLayout layout = std::vector<Hazel::BufferElement>{
			Hazel::BufferElement{ Hazel::ShaderDataType::Float2, "Position"},
			Hazel::BufferElement{ Hazel::ShaderDataType::Float3, "Color"},
			Hazel::BufferElement{ Hazel::ShaderDataType::Float2, "TexCoord"},
		};

		m_VertexBuffer->SetLayout(layout);	

		m_IndexBuffer = IndexBuffer::Create(indices.data(), indices.size());

		m_Shader = Shader::Create("assets/Shaders/vulkanShader.glsl");

		m_Material = MakeRef<VulkanMaterial>(m_Shader);
		m_Material->SetData("UniformBufferObject", m_UBO);
		m_Material->SetData("picture", m_Texture2D);
		
		

		FramebufferSpecification fbSpec;
		fbSpec.width = Application::GetInstance().GetWindow().GetWidth();
		fbSpec.height = Application::GetInstance().GetWindow().GetHeight();
		fbSpec.attachments = { TextureFormat::RGBA, TextureFormat::DEPTH24STENCIL8 };

		m_OffScreenFramebuffer = Framebuffer::Create(fbSpec);

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.shader = m_Shader;
		pipelineSpecification.bufferLayout = layout;		
		pipelineSpecification.targetFramebuffer = m_OffScreenFramebuffer;

		m_Pipeline = Pipeline::Create(pipelineSpecification);
		
		RenderPassSpecification renderPassSpec = {
			nullptr, m_Pipeline
		};
		m_RenderPass = RenderPass::Create(renderPassSpec);
		
	}

	void OnAttach() override {

	}

	void OnDetach() override {

	}

	void OnUpdate(Hazel::Timestep ts) override {
				
		// todo shouldn't exsist
		//Ref<VulkanCommandBuffer> commandBuffer = device->CreateCommandBuffer();
		 
		//commandBuffer->Begin();
		
		{

			
			// todo : where to go ?
			/*RenderCommand::BeginRenderPass(commandBuffer, m_RenderPass);
			RenderCommand::BindVertexBuffer(commandBuffer, m_VertexBuffer);
			RenderCommand::BindIndexBuffer(commandBuffer, m_IndexBuffer);

			RenderCommand::SubmitMaterial(commandBuffer, m_Pipeline, m_Material);
			RenderCommand::DrawIndexed(commandBuffer, indices.size());
			RenderCommand::EndRenderPass(commandBuffer, m_RenderPass);*/
			//
		}

		//commandBuffer->End();

		//commandBuffer->Submit();			
	}

	virtual void OnImGuiRender() override {

		ImGui::ShowDemoWindow();
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
	Ref<Framebuffer> m_OffScreenFramebuffer;
	Ref<RenderPass> m_RenderPass;
	Ref<Pipeline> m_Pipeline;	
	Ref<Shader> m_Shader;
	Ref<Texture2D> m_Texture2D;

	Ref<Material> m_Material;
		
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



