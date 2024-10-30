#include "SandBox2D.h"

#include "Hazel/Event/ApplicationEvent.h"

#include "imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Hazel/Renderer/Renderer2D.h"


SandBox2D::SandBox2D()
{
	m_SquareColor = glm::vec4(1.0f);
}

void SandBox2D::OnAttach()
{
	m_Camera = std::make_shared<Hazel::OrthographicCamera>(-1.6f, 1.6f, -1.0f, 1.0f);
	m_CameraController = std::make_shared<Hazel::OrthographicCameraController>(m_Camera, 1280.f / 720.f);

	m_Texture = Hazel::Texture2D::Create("assets/code.png");
}

void SandBox2D::OnDetach()
{
	
}

void SandBox2D::OnUpdate(Hazel::Timestep ts)
{

	Hazel::PROFILE_SCOPE("SandBox2D : OnUpdate");

	float deltaTime = ts;

	m_CameraController->OnUpdate(ts);

	Hazel::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	Hazel::RenderCommand::Clear();

	Hazel::Renderer2D::BeginScene(*m_Camera.get());
	Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.1f }, { 1.0f, 1.0f }, {1.0f, 1.0f, 0.0f, 1.0f});
	Hazel::Renderer2D::DrawQuad({0.0f, 0.25f}, {1.0f, 1.0f}, m_Texture);
	Hazel::Renderer2D::EndScene();
}

void SandBox2D::OnEvent(Hazel::Event& event)
{
	m_CameraController->OnEvent(event);
}

void SandBox2D::OnImGuiRender()
{
	ImGui::Begin("Setting");
	ImGui::ColorEdit4("Squre Color", glm::value_ptr(m_SquareColor));
	for (auto& result : m_ProfileResults) {
		char label[50];
		strcpy(label, result.name);
		strcat(label, " %.3fms");
		ImGui::Text(label, result.time);

	}
	ImGui::End();
	m_ProfileResults.clear();
}
