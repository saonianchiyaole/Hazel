#pragma once

#include "glm/glm.hpp"

#include "Hazel/Renderer/Camera.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Core/Timestep.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Hazel {

	struct TagComponent {
		std::string tag;
		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
		TagComponent(const std::string &tagValue) : tag(tagValue) {}
	};

	struct TransformComponent {
		glm::mat4 transform = glm::mat4(1.0f);
		glm::vec3 translate = {0.f, 0.f, 0.f};
		glm::vec3 scale = {1.f, 1.f, 1.f};
		glm::vec3 rotation = {0.f, 0.f, 0.f};

		TransformComponent() {
			transform = glm::mat4(1.0f);
		}
		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(const glm::mat4& value): transform(value){}
		TransformComponent(const glm::vec3& translateVal, const glm::vec3& scaleVal, const glm::vec3& rotationVal)
			: translate(translateVal), scale(scaleVal), rotation(rotationVal)
		{
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

		glm::mat4 GetTransform(){
			return transform;
		}
		void RecalculateTransform() {
			glm::mat4 rotationMat = glm::toMat4(glm::quat(rotation));
			transform = glm::translate(glm::mat4(1.0f), translate)*
				rotationMat*
				glm::scale(glm::mat4(1.0f), scale);
		}
		operator const glm::mat4& const() {
			return transform;
		}
	};

	struct SpriteComponent {
		glm::vec4 color;
		SpriteComponent():color(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f)){}
		SpriteComponent(const glm::vec4& colorVal) : color(colorVal) {}
		SpriteComponent(const SpriteComponent& other): color(other.color) {}
	};

	struct CameraComponent {
		Camera* camera;
		bool primary = false;
		CameraComponent() = default;
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

}