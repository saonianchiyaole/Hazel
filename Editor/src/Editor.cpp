#include "Editor.h"

#include <imgui.h>
#include "Hazel/Event/ApplicationEvent.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Hazel/Renderer/Renderer2D.h"
#include "entt.hpp"

namespace Hazel {

	Editor::Editor()
	{
		m_SquareColor = glm::vec4(1.0f);
	}

	void Editor::OnAttach()
	{
		m_Camera = std::make_shared<Hazel::Camera>();
		auto windowWidth = Hazel::Application::GetInstance().GetWindow().GetWidth();
		auto windowHeight = Hazel::Application::GetInstance().GetWindow().GetHeight();

		m_Camera->SetOrthographicProjection(1.0f, windowWidth / windowHeight, 0.1, 10000);
		//m_CameraController = std::make_shared<Hazel::OrthographicCameraController>(m_Camera, 1280.f / 720.f);

		m_Texture2 = Hazel::Texture2D::Create("assets/code.png");
		m_Texture1 = Hazel::Texture2D::Create("assets/Checkboard.png");
		m_Texture = Hazel::Texture2D::Create("assets/ChernoLogo.png");


		m_FramebufferSpecification.width = Hazel::Application::GetInstance().GetWindow().GetWidth();
		m_FramebufferSpecification.height = Hazel::Application::GetInstance().GetWindow().GetHeight();
		m_Framebuffer = Hazel::Framebuffer::Create(m_FramebufferSpecification);


		m_Scene.reset(new Scene);

		class CameraContorller : public Hazel::ScriptableEntity {
		public:
			void OnCreate() {
				std::cout << "CameraContorller Create!" << std::endl;
			}

			void OnDestroy() {

			}
			void OnUpdate(Timestep ts) {
				auto& transform = GetComponent<Hazel::TransformComponent>();
				float speed = 5.0f;

				if (Input::IsKeyPressed(HZ_KEY_A))
					transform.SetTranslate(glm::vec3{ transform.translate[0] - speed * ts, transform.translate[1], transform.translate[2] });
				if (Input::IsKeyPressed(HZ_KEY_D))
					transform.SetTranslate(glm::vec3{ transform.translate[0] + speed * ts, transform.translate[1], transform.translate[2] });
				if (Input::IsKeyPressed(HZ_KEY_W))
					transform.SetTranslate(glm::vec3{ transform.translate[0], transform.translate[1] - speed * ts, transform.translate[2] });
				if (Input::IsKeyPressed(HZ_KEY_S))
					transform.SetTranslate(glm::vec3{ transform.translate[0] , transform.translate[1] + speed * ts, transform.translate[2] });

			}
		};


		m_HierarchyPanel.SetContext(m_Scene);


		SceneSerializer sceneSerializer(m_Scene);
		sceneSerializer.Deserialize("assets\\Data\\data.yaml");

		// Init here
		m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
		m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
		m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
		m_Particle.LifeTime = 1.0f;
		m_Particle.Velocity = { 0.0f, 0.0f };
		m_Particle.VelocityVariation = { 3.0f, 1.0f };
		m_Particle.Position = { 0.0f, 0.0f };
	}

	void Editor::OnDetach()
	{

	}

	void Editor::OnUpdate(Hazel::Timestep ts)
	{

		Hazel::PROFILE_SCOPE("Editor : OnUpdate");

		float deltaTime = ts;

		m_Framebuffer->Bind();
		Hazel::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Hazel::RenderCommand::Clear();

		static float angle = 0.0f;
		angle += 1.f;

		m_Scene->OnUpdate(ts);

		/*m_ParticleSystem.OnUpdate(ts);
		m_ParticleSystem.OnRender(*m_Camera.get());
		*/
	
		
		m_Framebuffer->Unbind();

		Hazel::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Hazel::RenderCommand::Clear();

	}



	void Editor::OnImGuiRender()
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

				if (ImGui::MenuItem("Save")) {
					SceneSerializer sceneSerializer(m_Scene);
					sceneSerializer.Serialize("assets\\Data\\data.yaml");
				}
				if (ImGui::MenuItem("SaveAndClose")) {
					SceneSerializer sceneSerializer(m_Scene);
					sceneSerializer.Serialize("assets\\Data\\data.yaml");
					Application::GetInstance().Close();
				}

				if (ImGui::MenuItem("Close")) {
					Application::GetInstance().Close();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::End();

		ImGui::Begin("States");
		auto rendererState = Hazel::Renderer2D::GetState();

		ImGui::Text("DrawCall : %d", rendererState->drawCall);
		ImGui::Text("QuadAmount : %d", rendererState->quadAmount);
		ImGui::Text("VertexAmount : %d", rendererState->vertexAmount);
		ImGui::Text("IndexAmount : %d", rendererState->indexAmount);

		ImGui::End();

		ImGui::Begin("ViewPort");

		isViewportFocus = ImGui::IsWindowFocused();
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		if (viewportSize.x != 0 && viewportSize.y != 0) {
			if (m_ViewportSize != *(glm::vec2*)&viewportSize) {
				m_Framebuffer->Resize(glm::vec2{ viewportSize.x, viewportSize.y });
				m_Camera->SetAspectRatio(viewportSize.x / viewportSize.y);
				m_ViewportSize = { viewportSize.x, viewportSize.y };
				m_Scene->m_ViewPortSize = m_ViewportSize;
			}
			ImGui::Image((void*)m_Framebuffer->GetColorAttachment(), { (float)m_ViewportSize.x, (float)m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		}


		auto seletedEntity = m_HierarchyPanel.GetSelectedEntity();
		// ImGuizmo
		if (seletedEntity) {

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();


			float posx = ImGui::GetWindowPos().x;
			float posy = ImGui::GetWindowPos().y;
			float width = (float)ImGui::GetWindowWidth();
			float height = (float)ImGui::GetWindowHeight();


			ImGuizmo::SetRect(posx, posy, width, height);

			glm::vec3 translate, rotation, scale;

			auto primaryCamera = m_Scene->GetPrimaryCamera();
			if (primaryCamera) {

				auto view = primaryCamera->GetViewMatrix();
				auto projection = primaryCamera->GetProjectionMatrix();


				if (seletedEntity.HasComponent<TransformComponent>())
				{
					glm::mat4& transform = seletedEntity.GetComponent<TransformComponent>().transform;
					ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), m_ImGuizmoMode, ImGuizmo::MODE::LOCAL, glm::value_ptr(transform));
				}
			}




		}



		ImGui::End();


		m_HierarchyPanel.OnImGuiRender();

		m_ProfileResults.clear();
		Hazel::Renderer2D::ResetState();
	}


	void Editor::OnEvent(Hazel::Event& event)
	{
		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(Editor::OnMoustLeftButtonClicked));
		dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(Editor::OnWindowResized));
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(Editor::OnKeyPressed));
	}


	bool Editor::OnMoustLeftButtonClicked(Hazel::MouseButtonPressedEvent& event)
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

	bool Editor::OnWindowResized(Hazel::WindowResizeEvent& event)
	{
		return true;
	}

	bool Editor::OnKeyPressed(KeyPressedEvent& e)
	{

		switch (e.GetKeyCode())
		{
		case HZ_KEY_Q:
			m_ImGuizmoMode = (ImGuizmo::OPERATION)-1;
			break;
		case HZ_KEY_W:
			m_ImGuizmoMode = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case HZ_KEY_E:
			m_ImGuizmoMode = ImGuizmo::OPERATION::ROTATE;
			break;
		case HZ_KEY_R:
			m_ImGuizmoMode = ImGuizmo::OPERATION::SCALE;
			break;
		}
		return false;
	}

}