#include "hzpch.h"''
#include "Panels/SceneHierarchyPanel.h"
#include "Hazel/Scene/Component.h"
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Renderer/Camera.h"

namespace Hazel {
	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		m_Context = nullptr;
		m_SelectedEntity = {};
		m_EmptyTexture = Texture2D::Create("assets/Checkboard.png");
	}
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> context)
	{
		SetContext(context);
		m_SelectedEntity = {};

		m_EmptyTexture = Texture2D::Create("assets/Checkboard.png");
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene> context)
	{
		m_Context = context;
	}
	void SceneHierarchyPanel::SetSelectedEntity(const entt::entity entity, Ref<Scene> scene)
	{
		m_SelectedEntity = Entity{ entity, scene.get() };
		m_Context = scene;
	}
	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy Panel");

		auto view = m_Context->GetRegistry().view<TagComponent>();

		if (m_Context)
		{
			if (ImGui::BeginPopupContextWindow(0, 1)) {
				if (ImGui::MenuItem("Create New Entity")) {
					m_Context->CreateEntity("Empty Entity");
				}

				ImGui::EndPopup();
			}

			for (auto& entity : view) {
				Entity target{ entity,  m_Context->Raw() };
				DrawEntityNode(target);
			}

		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			m_SelectedEntity = {};
		}

		ImGui::End();


		ImGui::Begin("Properties");


		if (m_SelectedEntity)
		{
			DrawComponents(m_SelectedEntity);

			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");
			if (ImGui::BeginPopup("AddComponent")) {
				if (ImGui::MenuItem("Transform")) {
					m_SelectedEntity.AddComponent<TransformComponent>();
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Camera")) {
					m_SelectedEntity.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Sprite Renderer")) {
					m_SelectedEntity.AddComponent<SpriteComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Rigidbody 2D")) {
					m_SelectedEntity.AddComponent<Rigidbody2DComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Box Collider 2D")) {
					m_SelectedEntity.AddComponent<BoxCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::End();

	}
	void SceneHierarchyPanel::DrawEntityNode(Entity& entity) {

		auto& tag = entity.GetComponent<TagComponent>();
		ImGuiTreeNodeFlags flags = (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, tag.tag.c_str());

		if (ImGui::IsItemClicked()) {
			m_SelectedEntity = entity;
		}

		if (ImGui::BeginPopupContextItem()) {
			m_SelectedEntity = entity;
			if (ImGui::MenuItem("Delete")) {
				m_Context->DestroyEntity(m_SelectedEntity);
				m_SelectedEntity = {};
			}

			ImGui::EndPopup();
		}

		if (opened) {
			ImGui::TreePop();
		}
	}

	template<class T, class Function>
	void SceneHierarchyPanel::DrawComponent(Entity& entity, std::string componentName, Function func)
	{
		if (entity.HasComponent<T>()) {
			auto& component = entity.GetComponent<T>();

			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, componentName.c_str())) {

				func(component);

				ImGui::TreePop();

			}

		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity& entity)
	{
		if (entity.HasComponent<TagComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(TagComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Tag")) {
				auto& tag = entity.GetComponent<TagComponent>().tag;

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strcpy_s(buffer, sizeof(buffer), tag.c_str());
				if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
					tag = std::string(buffer);
				}
				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<TransformComponent>()) {
			auto& transform = entity.GetComponent<TransformComponent>();

			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {

				ImGui::TreePop();
				if (ImGui::DragFloat3("Position", glm::value_ptr(transform.translate), 0.1f)) {
					transform.RecalculateTransform();
					if (m_Context->GetState() == SceneState::Play && entity.HasComponent<Rigidbody2DComponent>())
					{
						Rigidbody2DComponent rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();
						rigidbody2D.SetPhysicsPosition({ transform.translate.x, transform.translate.y }, transform.rotation.z);
						if (!((b2Body*)rigidbody2D.runtimeBody)->IsAwake())
							((b2Body*)rigidbody2D.runtimeBody)->SetAwake(true);
						((b2Body*)rigidbody2D.runtimeBody)->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					}
						
				}

				glm::vec3 rotation = glm::degrees(transform.rotation);
				if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation))) {
					transform.SetRotation(glm::radians(rotation));
					transform.RecalculateTransform();
					if (m_Context->GetState() == SceneState::Play && entity.HasComponent<Rigidbody2DComponent>())
					{
						Rigidbody2DComponent rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();
						rigidbody2D.SetPhysicsPosition({ transform.translate.x, transform.translate.y }, transform.rotation.z);
						if (!((b2Body*)rigidbody2D.runtimeBody)->IsAwake())
							((b2Body*)rigidbody2D.runtimeBody)->SetAwake(true);
						((b2Body*)rigidbody2D.runtimeBody)->SetAngularVelocity(0.0f);
					}
				}
				if (ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.1f)) {
					transform.RecalculateTransform();
				}

			}

		}

		if (entity.HasComponent<CameraComponent>()) {
			auto& cameraComponent = entity.GetComponent<CameraComponent>();

			if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera")) {

				const char* projectionTypeString[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeString[(int)cameraComponent.camera->GetProjectionType()];

				if (ImGui::Checkbox("Primary", &cameraComponent.primary)) {



					// TODO : if set target camera's primary to true, set all the other camera's primary to false;
				}


				if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {
					for (int i = 0; i < 2; i++) {
						bool isSelected = currentProjectionTypeString == projectionTypeString[i];
						if (ImGui::Selectable(projectionTypeString[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeString[i];
							cameraComponent.camera->SetProjectionType((Camera::ProjectionType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (cameraComponent.camera->GetProjectionType() == Camera::ProjectionType::Perspective) {
					float fovy = cameraComponent.camera->GetFovy();
					if (ImGui::DragFloat("Fovy", &fovy)) {
						cameraComponent.camera->SetFovy(fovy);
					}
				}
				else if (cameraComponent.camera->GetProjectionType() == Camera::ProjectionType::Orthographic) {
					float zoomLevel = cameraComponent.camera->GetZoomLevel();
					if (ImGui::DragFloat("ZoomLevel", &zoomLevel)) {
						cameraComponent.camera->SetZoomLevel(zoomLevel);
					}
				}
				float nearClip = cameraComponent.camera->GetNearClip();
				if (ImGui::DragFloat("NearClip", &nearClip)) {
					cameraComponent.camera->SetNearClip(nearClip);
				}
				float farClip = cameraComponent.camera->GetFarClip();
				if (ImGui::DragFloat("FarClip", &farClip)) {
					cameraComponent.camera->SetFarClip(farClip);
				}


				ImGui::TreePop();
			}

		}

		if (entity.HasComponent<SpriteComponent>()) {
			auto& sprite = entity.GetComponent<SpriteComponent>();

			if (ImGui::TreeNodeEx((void*)typeid(SpriteComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer")) {

				ImGui::ColorEdit4("Color", glm::value_ptr(sprite.color));


				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

				if (sprite.texture == nullptr) {
					ImGui::ImageButton("##texture", (ImTextureID)m_EmptyTexture->GetRendererID(), { 50, 50 }, { 0, 1 }, { 1, 0 });
				}
				else {
					ImGui::ImageButton("##texture", (ImTextureID)sprite.texture->GetRendererID(), { 50, 50 }, { 0, 1 }, { 1, 0 });
				}
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
						const wchar_t* path = (const wchar_t*)payload->Data;
						sprite.texture = Texture2D::Create(path);
					}

					ImGui::EndDragDropTarget();
				}

				ImGui::PopStyleColor();


				ImGui::TreePop();

			}

		}

		DrawComponent<Rigidbody2DComponent>(entity, "Rigidbody 2D", [](auto& component)
			{
				const char* bodyTypeString[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeString[(int)component.type];
				if (ImGui::BeginCombo("Projection", currentBodyTypeString)) {
					for (int i = 0; i < 2; i++) {
						bool isSelected = currentBodyTypeString == bodyTypeString[i];
						if (ImGui::Selectable(bodyTypeString[i], isSelected))
						{
							currentBodyTypeString = bodyTypeString[i];
							component.type = (Rigidbody2DComponent::BodyType)i;
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.fixedRotation);
			});

		DrawComponent<BoxCollider2DComponent>(entity, "Box Collider 2D", [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.offset));
				ImGui::DragFloat2("Size", glm::value_ptr(component.size));
				ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
			});

	}
}