#include "hzpch.h"
#include "Hazel/Scene/Scene.h"

#include "Hazel/Scene/Entity.h"

#include "Hazel/Scene/Component.h"

#include "Hazel/Renderer/Renderer2D.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

namespace Hazel {

	static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType type) {
		switch (type) {
		case Rigidbody2DComponent::BodyType::Static:
			return b2BodyType::b2_staticBody;
		case Rigidbody2DComponent::BodyType::Dynamic:
			return b2BodyType::b2_dynamicBody;
		case Rigidbody2DComponent::BodyType::Kinematic:
			return b2BodyType::b2_kinematicBody;
		}
	}

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
				return cameraComponent.camera;
			}
		}

		return nullptr;
	}

	void Scene::OnRuntimeStart()
	{
		m_State = SceneState::Play;

		OnPhysics2DStart();

	}

	void Scene::OnRuntimeStop()
	{
		m_State = SceneState::Edit;
		OnPhysics2DStop();
	}

	void Scene::OnPhysics2DStart()
	{
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto entityID : view) {
			Entity entity = { entityID, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Rigidbody2DTypeToBox2DBody(rigidbody2D.type);
			bodyDef.position = { transform.translate.x, transform.translate.y };
			bodyDef.angle = transform.rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rigidbody2D.fixedRotation);
			rigidbody2D.runtimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>()) {
				auto& boxCollider2D = entity.GetComponent<BoxCollider2DComponent>();
				b2PolygonShape boxShape;
				boxShape.SetAsBox(boxCollider2D.size.x * transform.scale.x, boxCollider2D.size.y * transform.scale.y, b2Vec2(boxCollider2D.offset.x, boxCollider2D.offset.y), 0.0f);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = boxCollider2D.density;
				fixtureDef.friction = boxCollider2D.friction;
				fixtureDef.restitution = boxCollider2D.restitution;
				fixtureDef.restitutionThreshold = boxCollider2D.restitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::OnUpdateEditor(Timestep ts, const EditorCamera& camera)
	{

		Renderer2D::BeginScene(camera);

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
		for (auto entity : group) {
			auto& [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);

			Renderer2D::DrawSprite(transform, sprite, (int)entity);
		}

		Renderer2D::EndScene();

	}

	void Scene::OnUpdateRuntime(Timestep ts)
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

		//Physics
		{
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto entityID : view) {
				Entity entity = { entityID, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();

				b2Body* body = (b2Body*)rigidbody2D.runtimeBody;

				const auto& position = body->GetPosition();
				transform.translate.x = position.x;
				transform.translate.y = position.y;
				transform.rotation.z = body->GetAngle();
				transform.RecalculateTransform();
			
			}
		}

		Camera* mainCamera = nullptr;
		glm::mat4 mainCameraTransform = glm::mat4(1.0f);
		auto view = m_Registry.view<CameraComponent, TransformComponent>();
		for (auto& entity : view) {
			auto& [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);

			if (camera.primary == true) {
				mainCamera = camera.camera.get();
				mainCamera->SetTransform(transform.translate, transform.rotation);
			}
			break;
		}

		if (mainCamera != nullptr) {
			Renderer2D::BeginScene(*mainCamera);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
			for (auto entity : group) {
				auto& [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);
				Renderer2D::DrawSprite(transform, sprite, (int)entity);
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

	template<>
	void Scene::OnAddComponent<Rigidbody2DComponent>(Entity& entity, Rigidbody2DComponent& nativeScriptComponent) {
		if (!entity.HasComponent<TransformComponent>()) {
			entity.AddComponent<TransformComponent>();
		}
	}

	template<>
	void Scene::OnAddComponent<BoxCollider2DComponent>(Entity& entity, BoxCollider2DComponent& nativeScriptComponent) {
	}
}