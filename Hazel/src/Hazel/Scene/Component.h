#pragma once

#include "glm/glm.hpp"

#include "Hazel/Renderer/Camera.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/Mesh.h"
#include "Hazel/Renderer/Light.h"
#include "Hazel/Renderer/Material.h"


#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "box2d/b2_body.h"

namespace Hazel {

	struct IDComponent {
		UUID ID;
		IDComponent() {
		}
		IDComponent(uint64_t IDVal) : ID(IDVal) {}
		IDComponent(const IDComponent& other) = default;
	};


	struct TagComponent {
		std::string tag;
		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
		TagComponent(const std::string& tagValue) : tag(tagValue) {}
	};

	struct TransformComponent {
		glm::mat4 transform = glm::mat4(1.0f);
		glm::vec3 translate = { 0.f, 0.f, 0.f };
		glm::vec3 scale = { 1.f, 1.f, 1.f };
		glm::vec3 rotation = { 0.f, 0.f, 0.f };

		TransformComponent() {
		}
		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(const glm::mat4& value) : transform(value) {}
		TransformComponent(const glm::vec3& translateVal, const glm::vec3& rotationVal, const glm::vec3& scaleVal)
			: translate(translateVal), scale(scaleVal), rotation(rotationVal)
		{
			RecalculateTransform();
		}

		void SetTransform(glm::vec3 translationVal, glm::vec3 rotationVal, glm::vec3 scaleVal) {
			translate = translationVal;
			rotation = rotationVal;
			scale = scaleVal;
			RecalculateTransform();
		}

		void SetTranslate(glm::vec3 translateVal) {
			translate = translateVal;
			RecalculateTransform();
		}
		void SetScale(glm::vec3 scaleVal) {
			scale = scaleVal;
			RecalculateTransform();
		}
		void SetRotation(glm::vec3 rotationVal) {
			rotation = rotationVal;
			RecalculateTransform();
		}

		glm::mat4 GetTransform() {
			return transform;
		}
		void RecalculateTransform() {

			glm::mat4 rotationMat = glm::toMat4(glm::quat(rotation));

			transform = glm::translate(glm::mat4(1.0f), translate) *
				rotationMat *
				glm::scale(glm::mat4(1.0f), scale);
		}
		operator const glm::mat4& const() {
			return transform;
		}
	};

	struct SpriteComponent {
		glm::vec4 color;
		Ref<Texture2D> texture;
		SpriteComponent() :color(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f)) {}
		SpriteComponent(const glm::vec4& colorVal) : color(colorVal) {}
		SpriteComponent(const SpriteComponent& other) {
			color = other.color;
			texture = other.texture;
		}
	};

	struct CircleRendererComponent {
		glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
		float thickness = 1.0f;
		float fade = 0.005f;
		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent& other) = default;
	};

	struct CameraComponent {
		Ref<Camera> camera;
		bool primary = false;
		CameraComponent() { camera = MakeRef<Camera>(); }
		CameraComponent(Camera& cameraVal) : camera(&cameraVal) {}
		CameraComponent(Ref<Camera> camera) : camera(camera.get()) {}
		CameraComponent(const CameraComponent& other) = default;
	};

	struct NativeScriptComponent {
	private:

		friend class Scene;
		ScriptableEntity* instance = nullptr;

		std::function<ScriptableEntity* ()> InstantiateScript;
		std::function<void()> DestroyScript;


	public:
		template<typename T>
		void Bind() {
			InstantiateScript = [this]() { return new T(); };
			DestroyScript = [this]() { delete (T*)instance; instance = nullptr; };

		}
	};

	struct Rigidbody2DComponent {
		enum class BodyType { Static = 0, Dynamic = 1 , Kinematic = 2};
		BodyType type = BodyType::Static;
		bool fixedRotation = false;

		void* runtimeBody = nullptr;
		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent& other) = default;
		void SetPhysicsPosition(glm::vec2 position, float rotation) {
			((b2Body*)runtimeBody)->SetTransform({ position.x, position.y }, rotation);
		}
	};

	struct BoxCollider2DComponent {
		glm::vec2 offset = { 0.0f, 0.0f };
		glm::vec2 size = { 0.5f, 0.5f };

		float density = 1.0f;
		float friction = 0.5f;
		float restitution = 0.5f;
		float restitutionThreshold = 0.5f;


		void* runtimeFixture = nullptr;
		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent& other) = default;
	};

	struct CircleCollider2DComponent {
		glm::vec2 offset = { 0.0f, 0.0f };
		float radius = 0.5f;
		float density = 1.0f;
		float friction = 0.5f;
		float restitution = 0.5f;
		float restitutionThreshold = 0.5f;


		void* runtimeFixture = nullptr;
		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent& other) = default;
	};

	struct ScriptComponent {
		std::string scriptFilePath;
		std::string className;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other) = default;
	};

	struct MeshComponent {
		Ref<Mesh> mesh;
		MeshType type = Invalid;
		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;

		void SetMesh(Ref<Mesh> mesh) {
			this->mesh = mesh;
			type = mesh->GetMeshType();
		}
	};


	struct MaterialComponent {
		Ref<Material> material;
		std::string path;
		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent& other) = default;
		MaterialComponent(Ref<Material> materialVal) : material(materialVal){}

	};

	struct LightComponent {
		glm::vec3 color = {1.0f, 1.0f, 1.0f};
		LightType type = LightType::None;

		LightComponent() = default;
		LightComponent(const LightComponent& other) = default;
	};

	template<typename ... Components>
	struct ComponentGroup {

	};

	using AllComponents = ComponentGroup<TransformComponent, IDComponent, TagComponent, SpriteComponent,
		CircleRendererComponent, CameraComponent, NativeScriptComponent, Rigidbody2DComponent,
		BoxCollider2DComponent, CircleCollider2DComponent, ScriptComponent, MeshComponent, LightComponent, MaterialComponent>;

}