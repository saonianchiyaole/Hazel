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
				mainCamera->SetPosition(glm::vec3(transform.transform[3][0], transform.transform[3][1], transform.transform[3][2]));
			}
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

}