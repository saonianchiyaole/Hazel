#pragma once

#include "Hazel.h"
#include "Hazel/Renderer/OrthographicCameraController.h"
#include "Hazel/Core/Timer.h"
#include "Hazel/Renderer/ParticleSystem.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

#include "ImGuizmo.h"

namespace Hazel {
	class Editor : public Hazel::Layer {
	public:
		Editor();
		~Editor() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Hazel::Timestep ts) override;
		virtual void OnEvent(Hazel::Event& event) override;
		virtual void OnImGuiRender() override;

		bool OnMoustLeftButtonClicked(Hazel::MouseButtonPressedEvent& event);
		bool OnWindowResized(Hazel::WindowResizeEvent& event);
		bool OnKeyPressed(KeyPressedEvent& e);




		void OpenScene(std::filesystem::path filepath);
		
		void ProcessFile(std::filesystem::path filepath);

		//Function
		void UI_ToolBar();
		void ColliderVisiable();
		void DrawGuizmo();
		void SceneSetting();
		void DuplicateEntity();


	private:
		std::vector<Hazel::ProfileResult> m_ProfileResults;

		std::vector<Ref<Texture2D>> m_SkyboxTextures;
		Ref<TextureCube> m_SkyboxTextureCube;

		Ref<Environment> m_Environment;


		Hazel::Ref<Hazel::Framebuffer> m_Framebuffer;
		Hazel::FramebufferSpecification m_FramebufferSpecification;

		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		Ref<Scene> m_ActiveScene;
		SceneState m_SceneState;
		EditorCamera m_EditorCamera;

		SceneHierarchyPanel m_HierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		bool m_IsViewportFocus = false;
		bool m_IsViewportHovered = false;
		bool m_IsColliderVisiable = false;

		Hazel::ParticleProps m_Particle;
		Hazel::ParticleSystem m_ParticleSystem;

		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportButtomLeftPos;
		Entity m_SelectedEntity;
		std::string m_HoveredEntityString;

		float frameStartTime;

		Ref<Texture2D> m_PlayButtonTexture;
		Ref<Texture2D> m_PauseButtonTexture;

		ImGuizmo::OPERATION m_ImGuizmoMode = ImGuizmo::OPERATION::TRANSLATE;
	};
}