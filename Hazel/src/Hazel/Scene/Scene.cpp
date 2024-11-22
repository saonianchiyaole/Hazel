#include "hzpch.h"
#include "Hazel/Scene/Scene.h"

#include "Hazel/Scene/Entity.h"

#include "Hazel/Scene/Component.h"

#include "Hazel/Renderer/Renderer2D.h"

namespace Hazel {



	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };

		auto& tag = entity.AddComponent<TagComponent>();
		tag = name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	Ref<Camera> Scene::GetPrimaryCamera()
	{

		auto view = m_Registry.view<CameraComponent>();
		for (auto entityID : view) {
			Entity entity = { entityID, this };
			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			if (cameraComponent.primary) {
				return MakeRef<Camera>(*(entity.GetComponent<CameraComponent>().camera));
			}
		}

		return nullptr;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (!nsc.instance) {
					nsc.instance = nsc.InstantiateScript();
					nsc.instance->m_Entity = Entity{ entity, this };
					nsc.instance->OnCreate();
				}

				nsc.instance->OnUpdate(ts);
			}
		);


		Camera* mainCamera = nullptr;
		glm::mat4 mainCameraTransform = glm::mat4(1.0f);
		auto view = m_Registry.view<CameraComponent, TransformComponent>();
		for (auto& entity : view) {
			auto& [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);

			if (camera.primary == true) {
				mainCamera = camera.camera;
				mainCamera->SetTransform(transform.translate, transform.rotation);
			}
			break;
		}

		if (mainCamera != nullptr) {
			Renderer2D::BeginScene(*mainCamera);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
			for (auto entity : group) {
				auto& [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.color);
			}

			Renderer2D::EndScene();
		}
	}

	Scene* Scene::Raw()
	{
		return this;
	}

	template<class T>
	void Scene::OnAddComponent(Entity& entity, T& component) {
		static_assert(false);
	}

	template<>
	void Scene::OnAddComponent<TransformComponent>(Entity& entity, TransformComponent& transformComponent) {

	}

	template<>
	void Scene::OnAddComponent<CameraComponent>(Entity& entity, CameraComponent& cameraComponent) {
		cameraComponent.camera->SetAspectRatio(m_ViewPortSize.x / m_ViewPortSize.y);
	}
	template<>
	void Scene::OnAddComponent<TagComponent>(Entity& entity, TagComponent& tagComponent) {

	}
	template<>
	void Scene::OnAddComponent<SpriteComponent>(Entity& entity, SpriteComponent& spriteComponent) {

	}
	template<>
	void Scene::OnAddComponent<NativeScriptComponent>(Entity& entity, NativeScriptComponent& nativeScriptComponent) {

	}

}