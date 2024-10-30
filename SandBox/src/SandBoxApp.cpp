#include "Hazel.h"
#include "Hazel/EntryPoint.h"
#include "Hazel/Event/ApplicationEvent.h"

#include "imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Hazel/Renderer/OrthographicCameraController.h"

#include "SandBox2D.h"


class ExampleLayer : public Hazel::ImGuiLayer {
public:
	ExampleLayer()
		:ImGuiLayer() {

		m_ObjectPosition = glm::vec3(0.0f);
		m_SquareColor = glm::vec4(0.f);

		m_Camera.reset(new Hazel::OrthographicCamera(- 1.6f, 1.6f, -0.9f, 0.9f));
		m_CameraController.reset(new Hazel::OrthographicCameraController(m_Camera, 1280.f / 720.f));


		m_VertexArray = Hazel::VertexArray::Create();

		float vertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  //bottom left
			0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,  //bottom right
			0.5f,   0.5f, 0.0f, 1.0f, 1.0f,  //top right
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f   //top left
		};

		std::shared_ptr<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));


		Hazel::BufferLayout layout = std::vector<Hazel::BufferElement>{
			{ Hazel::ShaderDataType::Float3, "a_Position"},
			{ Hazel::ShaderDataType::Float2, "a_TexCrood"}
		};
		vertexBuffer->SetLayout(layout);


		m_VertexArray->AddVertexBuffer(vertexBuffer);


		unsigned int indices[6] = { 0, 1, 3, 3, 2, 1 };
		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(unsigned int)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		Hazel::ShaderLibrary::Load("assets/Shaders/Shader.glsl");
		m_Shader = Hazel::ShaderLibrary::Get("Shader");

		m_Texture = Hazel::Texture2D::Create("assets/ChernoLogo.png");

	}

	void OnAttach() override {

	}
	void OnDetach() override {

	}

	void OnUpdate(Hazel::Timestep ts) override {

		float deltaTime = ts;

		m_CameraController->OnUpdate(ts);

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_Shader)->UploadUniformInt("u_Texture", 0);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_ObjectPosition);

		m_Texture->Bind();


		//glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_ObjectPosition);

		//HZ_INFO("ExampleLayer::Update");
		Hazel::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Hazel::RenderCommand::Clear();



		Hazel::Renderer::BeginScene(*m_Camera.get());

		Hazel::Renderer::Submit(m_VertexArray, m_Shader, transform);

		Hazel::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override {

		ImGui::Begin("Setting");
		ImGui::ColorEdit4("Squre Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override {

		m_CameraController->OnEvent(event);

		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Hazel::KeyPressedEvent& e) {
		HZ_INFO("{0}", e);
		return false;
	}
private:
	Hazel::Ref<Hazel::Shader> m_Shader;
	Hazel::Ref<Hazel::VertexArray> m_VertexArray;
	Hazel::Ref<Hazel::OrthographicCamera> m_Camera;
	Hazel::Ref<Hazel::OrthographicCameraController> m_CameraController;

	float m_CameraMoveSpeed = 5.f;
	float m_CameraRotationSpeed = 30.f;
	float m_CameraRotationAngle = 0.0f;

	Hazel::Ref<Hazel::Texture> m_Texture;


	glm::vec3 m_ObjectPosition;
	glm::vec4 m_SquareColor;

};


class Sandbox :public Hazel::Application {
public:
	Sandbox() {
		//PushOverlay(new ExampleLayer());
		PushLayer(new SandBox2D());
	}
	~Sandbox() {

	}
};

Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}