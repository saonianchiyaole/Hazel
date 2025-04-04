#pragma once

#include "Hazel.h"
#include "Hazel/Renderer/OrthographicCameraController.h"
#include "Hazel/Core/Timer.h"
#include "Hazel/Renderer/ParticleSystem.h"


class SandBox2D : public Hazel::Layer {
public:
	SandBox2D();
	~SandBox2D() = default;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnEvent(Hazel::Event& event) override;
	virtual void OnImGuiRender() override;

	bool OnMoustLeftButtonClicked(Hazel::MouseButtonPressedEvent& event);
	bool OnWindowResized(Hazel::WindowResizeEvent& event);

private:
	Hazel::Ref<Hazel::OrthographicCameraController> m_CameraController;
	Hazel::Ref<Hazel::OrthographicCamera> m_Camera;
	Hazel::Ref<Hazel::Shader> m_Shader;
	Hazel::Ref<Hazel::VertexArray> m_VertexArray;
	glm::vec4 m_SquareColor;
	std::vector<Hazel::ProfileResult> m_ProfileResults;


	Hazel::Ref<Hazel::Texture2D> m_Texture;
	Hazel::Ref<Hazel::Texture2D> m_Texture1;
	Hazel::Ref<Hazel::Texture2D> m_Texture2;

	Hazel::Ref<Hazel::Framebuffer> m_Framebuffer;
	Hazel::FramebufferSpecification m_FramebufferSpecification;

	Hazel::ParticleProps m_Particle;
	Hazel::ParticleSystem m_ParticleSystem;

	glm::vec2 m_ViewportSize;
};