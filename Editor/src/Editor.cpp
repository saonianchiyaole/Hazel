#pragma once
#include "Editor.h"

#include <imgui.h>
#include "Hazel/Event/ApplicationEvent.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Hazel/Renderer/Renderer2D.h"
#include "entt.hpp"
#include "Hazel/UI/UI.h"
//#include "Hazel/Utils/PlatformUtils.h"
#include "Platform/Windows/WindowsUtils.cpp"


namespace Hazel {

	Editor::Editor()
	{
		m_SceneState = SceneState::Edit;
		m_ViewportSize = { 1280.f, 720.f };
	}

	void Editor::OnAttach()
	{

		m_PlayButtonTexture = Texture2D::Create("assets/ContentBrowserIcon/PlayButton.png");
		m_PauseButtonTexture = Texture2D::Create("assets/ContentBrowserIcon/PauseButton.png");

		m_Framebuffer = Renderer::GetGeometryPassFramebuffer();

		m_EditorScene.reset(new Scene);

		m_ActiveScene = m_EditorScene;

		m_HierarchyPanel.SetContext(m_ActiveScene);
		m_ContentBrowserPanel = { "assets" };

		m_EditorCamera = { 60.0f, 1280.f / 720.f, 0.001f, 1000.f };


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

		if (m_IsViewportHovered && m_SceneState == SceneState::Edit)
			m_EditorCamera.OnUpdate(ts);

		float deltaTime = ts;
		
		//m_Framebuffer->Bind();
		//Hazel::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		//Hazel::RenderCommand::Clear();

		int textureValue = -1;
		m_Framebuffer->ClearAttachment(1, (void*)&textureValue);

		switch (m_SceneState) {
		case SceneState::Edit:
			m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
			break;
		case SceneState::Play:
			m_ActiveScene->OnUpdateRuntime(ts);
			break;
		}

		ColliderVisiable();
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
				if (ImGui::MenuItem("Open...")) {
					std::string filePath = FileDialogs::OpenFile("Hazel Scene (*.scene)\0*.scene\0");
					OpenScene(filePath);
				}
				ImGui::Separator();

				if (ImGui::MenuItem("SaveAs...")) {
					std::string filePath = FileDialogs::SaveFile("Hazel Scene (*.scene)\0*.scene\0");
					if (!filePath.empty())
					{
						SceneSerializer sceneSerializer(m_EditorScene);
						sceneSerializer.Serialize(filePath);
					}
				}

				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					SceneSerializer sceneSerializer(m_EditorScene);
					sceneSerializer.Serialize("assets\\Data\\data.scene");
				}
				if (ImGui::MenuItem("SaveAndClose")) {
					SceneSerializer sceneSerializer(m_EditorScene);
					sceneSerializer.Serialize("assets\\Data\\data.scene");
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


		for (auto result : m_ProfileResults) {
			ImGui::Text("%s : %.5f ms", result.name, result.time);
		}

		ImGui::Text("DrawCall : %d", rendererState->drawCall);
		ImGui::Text("QuadAmount : %d", rendererState->quadAmount);
		ImGui::Text("VertexAmount : %d", rendererState->vertexAmount);
		ImGui::Text("IndexAmount : %d", rendererState->indexAmount);

		ImGui::End();


		ImGui::Begin("Setting");
		ImGui::Checkbox("Collider Visiable", &m_IsColliderVisiable);
		ImGui::End();

		ImGui::Begin("Shader");
		for (const auto& shader : ShaderLibrary::m_Shaders) {


			ImGui::Text("%s", shader.second->GetName().c_str());
			ImGui::SameLine();

			std::string buttonName = "Reload##" + shader.first;
			if (ImGui::Button(buttonName.c_str())) {
				shader.second->Reload();
			}
		}

		ImGui::End();


		// ViewPort Begin
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("ViewPort");

		m_IsViewportFocus = ImGui::IsWindowFocused();
		m_IsViewportHovered = ImGui::IsWindowHovered();
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		if (viewportSize.x != 0 && viewportSize.y != 0) {
			if (m_ViewportSize != *(glm::vec2*)&viewportSize) {
				m_Framebuffer->Resize(glm::vec2{ viewportSize.x, viewportSize.y });
				m_EditorCamera.SetViewportSize(viewportSize.x, viewportSize.y);
				m_ViewportSize = { viewportSize.x, viewportSize.y };
				m_ActiveScene->SetViewPortSize(m_ViewportSize);
				Renderer::SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			}
			Renderer::GetCompositePassFramebuffer()->Bind();
			ImGui::Image((void*)Renderer::GetCompositePassFramebuffer()->GetColorAttachment(0), {(float)m_ViewportSize.x, (float)m_ViewportSize.y}, ImVec2{0, 1}, ImVec2{1, 0});
			Renderer::GetCompositePassFramebuffer()->Unbind();
		}

		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* path = (const wchar_t*)payload->Data;
				
				ProcessFile(path);
			}

			ImGui::EndDragDropTarget();
		}


		m_ViewportButtomLeftPos = *(glm::vec2*)&ImGui::GetWindowPos();
		m_ViewportButtomLeftPos.y += ImGui::GetWindowHeight();


		UI_ToolBar();
		DrawGuizmo();


		ImGui::End();
		ImGui::PopStyleVar();
		// ViewPort End


		SceneSetting();

		m_HierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();


		m_ProfileResults.clear();
		Hazel::Renderer2D::ResetState();
	}


	void Editor::OnEvent(Hazel::Event& event)
	{
		if (m_IsViewportHovered && m_SceneState == SceneState::Edit)
			m_EditorCamera.OnEvent(event);

		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(Editor::OnMoustLeftButtonClicked));
		dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(Editor::OnWindowResized));
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(Editor::OnKeyPressed));
	}


	bool Editor::OnMoustLeftButtonClicked(Hazel::MouseButtonPressedEvent& event)
	{
		if (event.GetMouseButton() == HZ_MOUSE_BUTTON_LEFT && !Input::IsKeyPressed(HZ_KEY_LEFT_ALT) && m_IsViewportHovered && (!ImGuizmo::IsOver() || !m_HierarchyPanel.GetSelectedEntity())) {
			
			//Get Pixel value
			glm::vec2 mousePos = *(glm::vec2*)&ImGui::GetMousePos();
			mousePos = mousePos - m_ViewportButtomLeftPos;
			mousePos.y = -mousePos.y;
			//mousePos -= ImGui::GetStyle().WindowBorderSize;
			if (mousePos.x < 0 || mousePos.x > m_ViewportSize.x - 1
				|| mousePos.y < 0 || mousePos.y > m_ViewportSize.y - 1) {
				// Out of ViewPort
				m_SelectedEntity = Entity{};
			}
			else {
				//HZ_CORE_INFO("Mouse pos : {0} {1}", mousePos.x, mousePos.y);
				m_Framebuffer->Bind();
				int pixelVaule = Renderer::GetGeometryPassFramebuffer()->ReadPixel(1, mousePos.x, mousePos.y);
				m_SelectedEntity = pixelVaule == -1 ? Entity{} : Entity{ (entt::entity)pixelVaule, m_ActiveScene.get() };
				m_Framebuffer->Unbind();
			}

			m_HierarchyPanel.SetSelectedEntity(m_SelectedEntity);
		}

		/*auto [x, y] = Hazel::Input::GetMousePos();
		auto windowWidth = Hazel::Application::GetInstance().GetWindow().GetWidth();
		auto windowHeight = Hazel::Application::GetInstance().GetWindow().GetHeight();

		auto cameraWidth = m_EditorCamera.GetRight() - m_EditorCamera.GetLeft();
		auto cameraHeight = m_EditorCamera.GetTop() - m_EditorCamera.GetBottom();

		auto pos = m_EditorCamera.GetPosition();
		x = (x / windowWidth) * cameraWidth - cameraWidth * 0.5f;
		y = cameraHeight * 0.5f - (y / windowHeight) * cameraHeight;
		m_Particle.Position = { x + pos.x, y + pos.y };
		for (int i = 0; i < 5; i++)
			m_ParticleSystem.Emit(m_Particle);*/

		return false;
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

	void Editor::OpenScene(std::filesystem::path filepath)
	{
		HZ_CORE_INFO("Open Scene filePath {0}", filepath.string());
		
		m_EditorScene.reset(new Scene);
		m_ActiveScene = m_EditorScene;
		m_HierarchyPanel = { m_ActiveScene };

		SceneSerializer sceneSerializer(m_EditorScene);
		sceneSerializer.Deserialize(filepath.string());

		m_ActiveScene->SetViewPortSize(m_ViewportSize);
	}

	void Editor::ProcessFile(std::filesystem::path filepath)
	{
		static const std::set<std::string> supportedExtensions = { ".scene", ".hdr" };
		if (supportedExtensions.count(filepath.extension().string()) <= 0) {
			HZ_CORE_WARN("This is Not valid file!");
			return;
		}
		
		if (filepath.extension().string() == ".scene") {
			OpenScene(filepath);
		}
		else if (filepath.extension().string() == ".hdr") {
			m_ActiveScene->SetEnvironment(Environment::Create(filepath.string()));
		}
	}

	void Editor::UI_ToolBar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		bool toolbarEnabled = m_EditorScene != nullptr;

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
			tintColor.w = 0.5f;

		float size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		ImGui::BeginDisabled(!toolbarEnabled);

		switch (m_SceneState)
		{
		case Hazel::SceneState::Edit:
			if (ImGui::ImageButton("##PlayButton", (ImTextureID)m_PlayButtonTexture->GetRendererID(), { size, size }, { 0, 1 }, { 1, 0 })) {
				m_SceneState = SceneState::Play;

				m_RuntimeScene = Scene::CopyScene(m_EditorScene);
				m_ActiveScene = m_RuntimeScene;
				m_HierarchyPanel.SetContext(m_ActiveScene);
				m_RuntimeScene->OnRuntimeStart();
			}
			break;
		case Hazel::SceneState::Play:
			if (ImGui::ImageButton("##PauseButton", (ImTextureID)m_PauseButtonTexture->GetRendererID(), { size, size }, { 0, 1 }, { 1, 0 })) {
				m_SceneState = SceneState::Edit;

				m_RuntimeScene->OnRuntimeStop();
				m_RuntimeScene.reset();
				m_ActiveScene = m_EditorScene;
				m_HierarchyPanel.SetContext(m_ActiveScene);

			}
			break;
		default:
			break;
		}


		ImGui::EndDisabled();


		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void Editor::ColliderVisiable()
	{
		if (!m_IsColliderVisiable)
			return;
		Camera* camera = nullptr;
		switch (m_SceneState)
		{
		case Hazel::SceneState::Edit:
			camera = &m_EditorCamera;
			break;
		case Hazel::SceneState::Play:
			camera = m_ActiveScene->GetPrimaryCamera().get();
			if (!m_ActiveScene->GetPrimaryCamera())
				return;
			break;
		default:
			break;
		}
		Renderer2D::BeginScene(*camera);

		auto quadGroup = m_ActiveScene->m_Registry.view<TransformComponent, BoxCollider2DComponent>();
		for (auto entityID : quadGroup) {
			const auto& [transformComponent, boxCollider2D] = quadGroup.get<TransformComponent, BoxCollider2DComponent>(entityID);
			glm::mat4 transform = glm::translate(transformComponent.transform, { boxCollider2D.offset, 0.0f });
			Renderer2D::DrawRect(transform, { 0.0f, 1.0f, 0.0f, 1.0f }, (int)entityID);
		}

		auto circleGroup = m_ActiveScene->m_Registry.view<TransformComponent, CircleCollider2DComponent>();
		for (auto entityID : circleGroup) {
			const auto& [transformComponent, circleCollider2D] = circleGroup.get<TransformComponent, CircleCollider2DComponent>(entityID);
			glm::mat4 transform = glm::translate(transformComponent.transform, { circleCollider2D.offset, 0.0f });
			Renderer2D::DrawCircle(transform, { 0.0f, 1.0f, 0.0f, 1.0f }, 0.07f, 0.0025f, (int)entityID);
		}

		Renderer2D::EndScene();
	}

	void Editor::DrawGuizmo()
	{
		auto seletedEntity = m_HierarchyPanel.GetSelectedEntity();
		// ImGuizmo
		if (seletedEntity) {

			Camera* activeCamera = nullptr;
			switch (m_SceneState)
			{
			case Hazel::SceneState::Edit:
				activeCamera = &m_EditorCamera;
				break;
			case Hazel::SceneState::Play:
				activeCamera = m_ActiveScene->GetPrimaryCamera().get();
				break;
			default:
				break;
			}

			if (!activeCamera)
				return;


			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float posx = ImGui::GetWindowPos().x;
			float posy = ImGui::GetWindowPos().y;
			ImGuizmo::SetRect(posx, posy, m_ViewportSize.x, m_ViewportSize.y);


			bool snap = Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL);
			float snapValue = 0.5f;
			if (m_ImGuizmoMode == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			glm::vec3 translation, rotation, scale;

			const auto& view = activeCamera->GetViewMatrix();
			const auto& projection = activeCamera->GetProjectionMatrix();
			//auto 

			float snapValues[3] = { snapValue, snapValue, snapValue };


			if (seletedEntity.HasComponent<TransformComponent>())
			{
				glm::mat4 transform = seletedEntity.GetComponent<TransformComponent>().transform;
				ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), m_ImGuizmoMode, ImGuizmo::MODE::LOCAL,
					glm::value_ptr(transform), nullptr, snap ? &snapValues[0] : nullptr);

				if (ImGuizmo::IsUsing()) {
					Math::Decompose(transform, translation, rotation, scale);
					seletedEntity.GetComponent<TransformComponent>().SetTransform(translation, rotation, scale);
					if (m_SceneState == SceneState::Play && seletedEntity.HasComponent<Rigidbody2DComponent>())
					{
						Rigidbody2DComponent rigidbody2D = seletedEntity.GetComponent<Rigidbody2DComponent>();
						rigidbody2D.SetPhysicsPosition({ translation.x, translation.y }, rotation.z);
						if (!((b2Body*)rigidbody2D.runtimeBody)->IsAwake())
							((b2Body*)rigidbody2D.runtimeBody)->SetAwake(true);
						((b2Body*)rigidbody2D.runtimeBody)->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					}
				}

			}
		}
	}

	void Editor::SceneSetting()
	{
		//Scene Setting
		ImGui::Begin("Scene Setting");

		static bool isSkyboxSettingShowned = false;

		if (ImGui::Button("Set Skybox")) {
			isSkyboxSettingShowned = true;
		}

		if (isSkyboxSettingShowned)
		{
			ImGui::Begin("Skybox Setting");

			Ref<TextureCube> textureCube = m_ActiveScene->GetSkybox();

			std::vector<Ref<Texture2D>> textures;
			if (textureCube)
				textures = textureCube->GetTexutres();

			std::function<void(Ref<Texture2D>&, void*)> dragDropFunc = [&](Ref<Texture2D>& texture, void* data) {
				const wchar_t* rawPath = (const wchar_t*)data;
				std::filesystem::path path(rawPath);
				texture = TextureLibrary::Load(path.string());
				};

			const std::string lable[6] = {
				"Right\n(+X)", "Left\n(-X)", "Up\n(+Y)", "Down\n(-Y)", "Front\n(+Z)", "Back\n(-Z)"
			};

			uint32_t emptyRendererID = m_HierarchyPanel.m_EmptyTexture->GetRendererID();
			ImGui::Columns(2, 0, false);

			for (size_t i = 0; i < 6; i++) {

				std::string strID = lable[i];
				uint32_t rendererID = (textures.empty() || !textures[i]) ? emptyRendererID : textures[i]->GetRendererID();;
				ImGui::ImageButton(strID.c_str(), (ImTextureID)rendererID, { 50, 50 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						const wchar_t* rawPath = (const wchar_t*)payload->Data;
						std::filesystem::path path(rawPath);
						if (!m_ActiveScene->GetSkybox()) {
							m_ActiveScene->SetSkybox(TextureCube::Create());
						}
						m_ActiveScene->GetSkybox()->SetTexture(TextureLibrary::Load(path.string()), i);
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::TextWrapped(strID.c_str());			
				ImGui::NextColumn();
						
			}
			ImGui::Columns(1);


			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 buttonSize = ImVec2(150, 30);

			ImGui::SetCursorPosX(windowSize.x / 2 - buttonSize.x / 2);
			if (ImGui::Button("Done", buttonSize)) {
				isSkyboxSettingShowned = false;
			}

			ImGui::End();


		}



		ImGui::End();
		//Scene Setting End
	}

}