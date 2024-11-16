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

	m_Texture2 = Hazel::Texture2D::Create("assets/code.png");
	m_Texture1 = Hazel::Texture2D::Create("assets/Checkboard.png");
	m_Texture = Hazel::Texture2D::Create("assets/ChernoLogo.png");


    m_FramebufferSpecification.width = Hazel::Application::GetInstance().GetWindow().GetWidth();
    m_FramebufferSpecification.height = Hazel::Application::GetInstance().GetWindow().GetHeight();
    m_Framebuffer = Hazel::Framebuffer::Create(m_FramebufferSpecification);
   

	// Init here
	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 1.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };
}

void SandBox2D::OnDetach()
{

}

void SandBox2D::OnUpdate(Hazel::Timestep ts)
{

	Hazel::PROFILE_SCOPE("SandBox2D : OnUpdate");

	float deltaTime = ts;

	m_CameraController->OnUpdate(ts);
    m_Framebuffer->Bind();
	Hazel::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	Hazel::RenderCommand::Clear();

	static float angle = 0.0f;
	angle += 1.f;


	Hazel::Renderer2D::BeginScene(*m_Camera.get());
	Hazel::Renderer2D::DrawQuad({ -1.f, 0.0f, -0.25f }, { 0.5f, 0.5f }, m_Texture);
	Hazel::Renderer2D::DrawRotatedQuad({ -0.25f, 0.0f, -0.1f }, { 1.0f, 1.0f }, angle, m_Texture1);
	Hazel::Renderer2D::DrawRotatedQuad({ 0.25f, 0.25f }, { 0.5f, 1.0f }, 0.0f, { 0.5f, 1.0f, 0.f, 1.f });
	Hazel::Renderer2D::DrawQuad({ 0.5f, 0.25f }, { 0.25f, 1.0f }, { 0.5f, 1.0f, 1.f, 1.f });

	Hazel::Renderer2D::EndScene();

	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(*m_Camera.get());
    m_Framebuffer->Unbind();

    Hazel::Renderer2D::BeginScene(*m_Camera.get());
    Hazel::Renderer2D::DrawRotatedQuad({ 0.25f, 0.25f }, { 0.5f, 1.0f }, 0.0f, { 0.5f, 1.0f, 0.f, 1.f });
    Hazel::Renderer2D::DrawQuad({ 0.5f, 0.25f }, { 0.25f, 1.0f }, { 0.5f, 1.0f, 1.f, 1.f });

    Hazel::Renderer2D::EndScene();
	
}

void SandBox2D::OnEvent(Hazel::Event& event)
{
	m_CameraController->OnEvent(event);

	Hazel::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Hazel::MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(SandBox2D::OnMoustLeftButtonClicked));
	dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(SandBox2D::OnWindowResized));

}

void SandBox2D::OnImGuiRender()
{

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
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
            if (ImGui::MenuItem("Flag: NoDockingSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
            if (ImGui::MenuItem("Flag: NoUndocking", "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
            if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            ImGui::Separator();

            if (ImGui::MenuItem("Close", NULL, false, m_Open != NULL))
                m_Open = false;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();

	ImGui::Begin("Setting");
	ImGui::ColorEdit4("Squre Color", glm::value_ptr(m_SquareColor));
	for (auto& result : m_ProfileResults) {
		char label[50];
		strcpy_s(label, result.name);
		strcat(label, " %.3fms");
		ImGui::Text(label, result.time);

	}
	auto rendererState = Hazel::Renderer2D::GetState();

	ImGui::Text("DrawCall : %d", rendererState->drawCall);
	ImGui::Text("QuadAmount : %d", rendererState->quadAmount);
	ImGui::Text("VertexAmount : %d", rendererState->vertexAmount);
	ImGui::Text("IndexAmount : %d", rendererState->indexAmount);

	

	ImGui::End();

    ImGui::Begin("ViewPort");
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (m_ViewportSize != *(glm::vec2*)&viewportSize) {
        m_Framebuffer->Resize(glm::vec2{ viewportSize.x, viewportSize.y });
        m_CameraController->SetAspectRatio(viewportSize.x / viewportSize.y);
        m_ViewportSize = { viewportSize.x, viewportSize.y };
    }
    ImGui::Image((void*)m_Framebuffer->GetColorAttachment(), { (float)m_ViewportSize.x, (float)m_ViewportSize.y });
    ImGui::End();


	m_ProfileResults.clear();
	Hazel::Renderer2D::ResetState();
}

bool SandBox2D::OnMoustLeftButtonClicked(Hazel::MouseButtonPressedEvent& event)
{
    HZ_INFO("Mouse Position {0},{1}", Hazel::Input::GetMousePos().first, Hazel::Input::GetMousePos().second);
	if (event.GetMouseButton() != HZ_MOUSE_BUTTON_LEFT)
		return false;
	auto [x, y] = Hazel::Input::GetMousePos();
	auto windowWidth = Hazel::Application::GetInstance().GetWindow().GetWidth();
	auto windowHeight = Hazel::Application::GetInstance().GetWindow().GetHeight();

	auto cameraWidth = m_Camera->GetRight() - m_Camera->GetLeft();
	auto cameraHeight = m_Camera->GetTop() - m_Camera->GetBottom();

	auto pos = m_Camera->GetPosition();
	x = (x / windowWidth) * cameraWidth - cameraWidth * 0.5f;
	y = cameraHeight * 0.5f - (y / windowHeight) * cameraHeight;
	m_Particle.Position = { x + pos.x, y + pos.y };
	for (int i = 0; i < 5; i++)
		m_ParticleSystem.Emit(m_Particle);

	return true;
}

bool SandBox2D::OnWindowResized(Hazel::WindowResizeEvent& event)
{
    return true;
}