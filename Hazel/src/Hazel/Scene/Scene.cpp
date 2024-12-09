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

	static std::unordered_map<UUID, std::pair<entt::entity, entt::entity>> s_CopyRegistry;


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

	Ref<Scene> Scene::CopyScene(Ref<Scene> src)
	{
		s_CopyRegistry.clear();

		Ref<Scene> dst;
		dst.reset(new Scene);

		const auto& srcRegistry = src->GetRegistry();
		const auto& dstRegistry = dst->GetRegistry();

		auto srcView = srcRegistry.view<entt::entity>();
		for (auto entityID : srcView) {
			const auto& tag = srcRegistry.get<TagComponent>(entityID).tag;
			auto uuid = srcRegistry.get<IDComponent>(entityID).ID;

			s_CopyRegistry[uuid] = { entityID, dst->CreateEnttiyWithUUID(tag, uuid).GetHandle() };

			CopyComponent<TransformComponent>(src, dst, uuid);
			CopyComponent<SpriteComponent>(src, dst, uuid);
			CopyComponent<CircleRendererComponent>(src, dst, uuid);
			CopyComponent<CameraComponent>(src, dst, uuid);
			CopyComponent<NativeScriptComponent>(src, dst, uuid);
			CopyComponent<Rigidbody2DComponent>(src, dst, uuid);
			CopyComponent<BoxCollider2DComponent>(src, dst, uuid);
			CopyComponent<CircleCollider2DComponent>(src, dst, uuid);

		}

		return dst;
	}


	template<class Component>
	void Scene::CopyComponent(Ref<Scene> src, Ref<Scene> dst, UUID uuid)
	{
		auto view = src->GetRegistry().view<Component>();
		const auto& srcRegistry = src->GetRegistry();
		if (srcRegistry.all_of<Component>(s_CopyRegistry[uuid].first)) {
			auto component = src->GetRegistry().get<Component>(s_CopyRegistry[uuid].first);
			dst->GetRegistry().emplace_or_replace<Component>(s_CopyRegistry[uuid].second, component);
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEnttiyWithUUID(name, UUID());
	}

	Entity Scene::CreateEnttiyWithUUID(const std::string& name, const uint64_t ID)
	{
		Entity entity{ m_Registry.create(), this };

		entity.AddComponent<IDComponent>(ID);
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

			if (entity.HasComponent<CircleCollider2DComponent>()) {
				auto& circleCollider2D = entity.GetComponent<CircleCollider2DComponent>();
				b2CircleShape circleShape;
				circleShape.m_p.Set(circleCollider2D.offset.x, circleCollider2D.offset.y);
				circleShape.m_radius = transform.scale.x * circleCollider2D.radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = circleCollider2D.density;
				fixtureDef.friction = circleCollider2D.friction;
				fixtureDef.restitution = circleCollider2D.restitution;
				fixtureDef.restitutionThreshold = circleCollider2D.restitutionThreshold;
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

		auto circleGroup = m_Registry.view<TransformComponent, CircleRendererComponent>();
		for (auto entityID : circleGroup) {
			auto& [transform, circle] = circleGroup.get<TransformComponent, CircleRendererComponent>(entityID);
			Renderer2D::DrawCircle(transform, circle.color, circle.thickness, circle.fade, (int)entityID);
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

			auto quadGroup = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
			for (auto entityID : quadGroup) {
				auto& [transform, sprite] = quadGroup.get<TransformComponent, SpriteComponent>(entityID);
				Renderer2D::DrawSprite(transform, sprite, (int)entityID);
			}

			auto circleGroup = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entityID : circleGroup) {
				auto& [transform, circle] = circleGroup.get<TransformComponent, CircleRendererComponent>(entityID);
				Renderer2D::DrawCircle(transform, circle.color, circle.thickness, circle.fade, (int)entityID);
			}



			Renderer2D::EndScene();
		}
	}

	void Scene::SetViewPortSize(glm::vec2 viewPortSize)
	{
		m_ViewPortSize = viewPortSize;
		if (GetPrimaryCamera() != nullptr)
			GetPrimaryCamera()->SetAspectRatio(viewPortSize.x / viewPortSize.y);
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
	void Scene::OnAddComponent<IDComponent>(Entity& entity, IDComponent& transformComponent) {

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
	void Scene::OnAddComponent<CircleRendererComponent>(Entity& entity, CircleRendererComponent& nativeScriptComponent) {
	}

	template<>
	void Scene::OnAddComponent<BoxCollider2DComponent>(Entity& entity, BoxCollider2DComponent& nativeScriptComponent) {
	}

	template<>
	void Scene::OnAddComponent<CircleCollider2DComponent>(Entity& entity, CircleCollider2DComponent& nativeScriptComponent) {
	}
}