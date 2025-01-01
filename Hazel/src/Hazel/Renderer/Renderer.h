#pragma once

#include "Hazel/Renderer/RenderCommand.h"

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Camera.h"

#include "Hazel/Renderer/Mesh.h"
#include "Hazel/Renderer/EditorCamera.h"

#include "Hazel/Scene/Component.h"


namespace Hazel {
	
	struct TransformComponent;



	struct LightUniformBuffer {
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 color;
	};

	struct CameraUniformBuffer {
		glm::mat4 viewProjectionMatrix;
		glm::vec4 position;
	};

	class Renderer {
	public:

		static void BeginScene(const Camera& camera);
		static void BeginScene(const EditorCamera& camera);

		static void EndScene();

		static void Submit(const Ref<VertexArray>& vertexArray, Ref<Shader>& shader, const glm::mat4& transform = glm::mat4(1.0f));

		static void Init();

		static void SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent = TransformComponent(), Ref<Shader> shader = s_SceneData->defaultShader, UUID entityID = -1);

		static void SubmitLight(const LightComponent& lightComponent, const TransformComponent& transformComponent);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void OnWindowResize(uint32_t width, uint32_t height);

	private:
		struct SceneData {
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewProjectionMatrix;

			Ref<Shader> defaultShader;

			Ref<UniformBuffer> cameraUniformBuffer;
			Ref<UniformBuffer> lightUniformBuffer;

		};
		static SceneData* s_SceneData;
	};

}