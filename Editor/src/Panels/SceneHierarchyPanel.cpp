#include "hzpch.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Hazel/Scene/Component.h"
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Renderer/Camera.h"
#include "Hazel/Scripting/ScriptEngine.h"

#include "mono/metadata/class.h"

//#include "Platform/Windows/WindowsUtils.h"
#include "Hazel/Utils/PlatformUtils.h"

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
		m_SelectedEntity = {};
	}
	void SceneHierarchyPanel::SetSelectedEntity(const entt::entity entity)
	{
		m_SelectedEntity = Entity{ entity, m_Context.get() };
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

				if (ImGui::MenuItem("Circle Renderer")) {
					m_SelectedEntity.AddComponent<CircleRendererComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Mesh")) {
					m_SelectedEntity.AddComponent<MeshComponent>();
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

				if (ImGui::MenuItem("Circle Collider 2D")) {
					m_SelectedEntity.AddComponent<CircleCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Script")) {
					m_SelectedEntity.AddComponent<ScriptComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Light")) {
					m_SelectedEntity.AddComponent<LightComponent>();
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


		DrawComponent<ScriptComponent>(entity, "Script", [&](auto& component)
			{
				bool isEntityClassExist = ScriptEngine::IsEntityClassExist(component.className);

				static char buffer[64];
				strcpy_s(buffer, sizeof(buffer), component.className.c_str());


				if (!isEntityClassExist) {

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));
					if (ImGui::InputText("Class", buffer, sizeof(buffer))) {
						component.className = buffer;
					}
					ImGui::PopStyleColor();
				}
				else {
					if (ImGui::InputText("Class", buffer, sizeof(buffer))) {
						component.className = buffer;
					}
				}

				// Draw Fields 
				if (m_Context->GetState() == SceneState::Play && isEntityClassExist) {
					const auto& fields = ScriptEngine::GetScriptClass(component.className)->GetFields();
					UUID entityID = entity.GetEntityID();
					Ref<ScriptInstance> instance = ScriptEngine::GetScriptInstance(entityID);
					for (auto field : fields) {
						switch (field.second.type) {
						case ScriptFieldType::None:
							continue;
						case ScriptFieldType::Float:
						case ScriptFieldType::Double:
						{
							float data = instance->GetFieldValue<float>(field.first);
							if (ImGui::InputFloat(field.first.c_str(), &data))
								instance->SetFieldVaule(field.first, data);
						}
						break;
						case ScriptFieldType::Bool:
							//ImGui::Checkbox(field.second.name, );
							break;
						case ScriptFieldType::Char:
							//ImGui::InputText(field.second.name, )
							break;
						case ScriptFieldType::Short:
						case ScriptFieldType::Int:
						case ScriptFieldType::Long:
						case ScriptFieldType::Byte:
						case ScriptFieldType::UShort:
							break;
						case ScriptFieldType::UInt:
							break;
						case ScriptFieldType::ULong:

							//ImGui::InputInt(field.second.name, );
							break;
						case ScriptFieldType::Vector2:
						{
							glm::vec2 data = instance->GetFieldValue<glm::vec2>(field.first);
							if (ImGui::InputFloat2(field.first.c_str(), glm::value_ptr(data)))
								instance->SetFieldVaule(field.first, data);
						}
						//ImGui::InputFloat2(field.second.name, );
						break;
						case ScriptFieldType::Vector3:
							//ImGui::InputFloat3(field.second.name, );
							break;
						case ScriptFieldType::Vector4:
							//ImGui::InputFloat4(field.second.name, );
							break;
						case ScriptFieldType::Entity:
							break;
						}
					}
				}

			});

		DrawComponent<MeshComponent>(entity, "Mesh", [&](auto& component)
			{
				auto& meshComponent = entity.GetComponent<MeshComponent>();

				std::string context;

				if (meshComponent.mesh == nullptr) {
					context = "Select Mesh";
				}
				else {
					context = meshComponent.mesh->GetFilePath();
					size_t pos = context.find_last_of("\\");
					if (pos != std::string::npos) {
						context = context.substr(pos + 1);
					}
				}

				if (ImGui::Button(context.c_str())) {
					std::string filePath = FileDialogs::OpenFile("Hazel Scene (*.fbx)\0*.fbx\0");
					meshComponent.SetMesh(MakeRef<Mesh>(filePath, entity.GetHandle()));
				}

			}
		);

		DrawComponent<CircleRendererComponent>(entity, "Circle Renderer", [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
				ImGui::DragFloat("Thickness", &component.thickness, 0.025, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.fade, 0.0025, 0.00, 1.0f);
			});

		DrawComponent<Rigidbody2DComponent>(entity, "Rigidbody 2D", [](auto& component)
			{
				const char* bodyTypeString[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeString[(int)component.type];
				if (ImGui::BeginCombo("BodyType", currentBodyTypeString)) {
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

		DrawComponent<CircleCollider2DComponent>(entity, "Circle Collider 2D", [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.offset));
				ImGui::DragFloat("Radius", &component.radius);
				ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
			});

		DrawComponent<LightComponent>(entity, "Light", [&](auto& component)
			{
				auto& light = entity.GetComponent<LightComponent>();
				ImGui::DragFloat3("Color", glm::value_ptr(light.color), 0.05f, 0.0f, 1.0f);
			}
		);

	}
}