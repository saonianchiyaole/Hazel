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

#include "backends/imgui_impl_vulkan.h"


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
		m_UBO.view = glm::mat4(1.0f);//m_Camera.GetViewMatrix();
		m_UBO.projection = glm::mat4(1.0f);

		RenderCommand::Init();

		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < m_CommandBuffers.size(); i++) {
			m_CommandBuffers[i] = CommandBuffer::Create();
		}

		m_Texture2D = TextureLibrary::Load("assets/Checkboard.png");


		m_VertexArray = VertexArray::Create();
		
		m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));

		Hazel::BufferLayout layout = std::vector<Hazel::BufferElement>{
			Hazel::BufferElement{ Hazel::ShaderDataType::Float2, "Position"},
			Hazel::BufferElement{ Hazel::ShaderDataType::Float3, "Color"},
			Hazel::BufferElement{ Hazel::ShaderDataType::Float2, "TexCoord"},
		};

		m_VertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		m_IndexBuffer = IndexBuffer::Create(indices.data(), indices.size());
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

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

				
		uint32_t currentFrameIndex = Application::GetInstance().GetWindow().GetSwapchain()->GetCurrentFrameIndex();
		Ref<CommandBuffer> commandBuffer = m_CommandBuffers[currentFrameIndex];

		/*commandBuffer->Begin();

		{

			RenderCommand::BeginRenderPass(commandBuffer, m_RenderPass);		
			RenderCommand::SubmitMaterial(commandBuffer, m_RenderPass->GetPipeline(), m_Material);
			RenderCommand::DrawIndexed(commandBuffer, m_VertexArray);
			RenderCommand::EndRenderPass(commandBuffer, m_RenderPass);
			
		}

		commandBuffer->End();

		commandBuffer->Submit();	*/		
	}

	virtual void OnImGuiRender() override {


		ImGui::ShowDemoWindow();

		static bool m_Open = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &m_Open, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				if (ImGui::MenuItem("Open...")) {

				}
				ImGui::Separator();

				if (ImGui::MenuItem("SaveAs...")) {

				}

				if (ImGui::MenuItem("Save", "Ctrl+S")) {

				}
				if (ImGui::MenuItem("SaveAndClose")) {

				}

				if (ImGui::MenuItem("Close")) {

				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}


		ImGui::End();



		ImGui::Begin("ViewPort");


		uint8_t frameIndex = Application::GetInstance().GetWindow().GetSwapchain()->GetCurrentFrameIndex();

		Ref<VulkanTexture2D> texture = std::static_pointer_cast<VulkanTexture2D>(m_OffScreenFramebuffer->GetColorAttachment(0));
		if (!m_ImGuiDescriptorSet) {			
			m_ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(texture->GetSampler(), texture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);			
		}

		//Utils::TransitionImageLayout(texture->GetRawImage(), Utils::GetVulkanFormatFromTextureFormat(texture->GetTextureFormat()), texture->GetLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		if (viewportSize.x != 0 && viewportSize.y != 0) {
			if (m_ViewportSize != *(glm::vec2*)&viewportSize) {
				/*m_Framebuffer->Resize(glm::vec2{ viewportSize.x, viewportSize.y });
				m_EditorCamera.SetViewportSize(viewportSize.x, viewportSize.y);
				m_ViewportSize = { viewportSize.x, viewportSize.y };
				m_ActiveScene->SetViewPortSize(m_ViewportSize);
				Renderer::SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);*/
				m_ViewportSize = { viewportSize.x, viewportSize.y };
			}
			

			
			//ImGui::Image(m_ImGuiDescriptorSet, *(ImVec2*)&m_ViewportSize);
		}

		ImGui::End();
		
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
	Ref<VertexArray> m_VertexArray;
	Ref<VertexBuffer> m_VertexBuffer;
	Ref<IndexBuffer> m_IndexBuffer;

	std::vector<Ref<CommandBuffer>> m_CommandBuffers;

	Ref<Framebuffer> m_OffScreenFramebuffer;
	Ref<RenderPass> m_RenderPass;
	Ref<Pipeline> m_Pipeline;
	Ref<Shader> m_Shader;
	Ref<Texture2D> m_Texture2D;

	Ref<Material> m_Material;

	VkDescriptorSet m_ImGuiDescriptorSet = nullptr;

	Camera m_Camera;
	UniformBufferObject m_UBO;

	glm::vec2 m_ViewportSize = { 0,0 };

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



