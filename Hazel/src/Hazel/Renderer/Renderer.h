#pragma once

#include "Hazel/Renderer/RenderCommand.h"

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Camera.h"

#include "Hazel/Renderer/Mesh.h"
#include "Hazel/Renderer/EditorCamera.h"

#include "Hazel/Scene/Component.h"
#include "Hazel/Renderer/RenderPass.h"

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

	struct DrawCommand {
		Ref<Mesh> mesh;
		Ref<Material> material;
		glm::mat4 transform;
	};

	// this should be Scene Renderer
	class Renderer {
	public:

		static void BeginScene(const Camera& camera);
		static void BeginScene(const EditorCamera& camera);

		static void EndScene();

		static void Submit(const Ref<VertexArray>& vertexArray, Ref<Shader>& shader, const glm::mat4& transform = glm::mat4(1.0f));

		static void Init();

		static void SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent = TransformComponent(), Ref<Shader> shader = s_SceneData->defaultShader, UUID entityID = -1);
		static void SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, Ref<Material> material);

		static void SubmitLight(const LightComponent& lightComponent, const TransformComponent& transformComponent);
		static void SubmitSkybox(Ref<TextureCube> skyboxTextures);
		static void SubmitEnvironment(Ref<Environment> environment);
		

		static uint32_t GetNextEmptyTextureSlot();

		static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = false);
		static void EndRenderPass();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }


		//RenderPass Function
		static void GeometryPass();
		static void CompositePass();

		static void FlushDrawList();
		static Ref<Framebuffer> GetGeometryPassFramebuffer();
		static Ref<Framebuffer> GetCompositePassFramebuffer();

		static void SetViewportSize(uint32_t width, uint32_t height);
		static void OnWindowResize(uint32_t width, uint32_t height);
		static Ref<Material> GetDefaultPBRMaterial();
		static Ref<Material> GetDefaultPhongMaterial();
	private:
		
		struct SceneData {
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewProjectionMatrix;

			Ref<Shader> defaultShader;
			Ref<Shader> skyboxShader;
			Ref<Shader> compositeShader;

			Ref<Material> defaultPBRMaterial;
			Ref<Material> defaultPhongMaterial;
			Ref<Material> skyboxMaterial;

			Ref<Camera> primaryCamera;

			Ref<UniformBuffer> cameraUniformBuffer;
			Ref<UniformBuffer> lightUniformBuffer;
			Ref<VertexArray> skybox;
			Ref<Environment> environment;

			Ref<VertexArray> fullScreenQuad;
			
			
			const uint32_t maxTextureSlot = 32;
			uint32_t textureSlotIndex;

			Ref<RenderPass> geometryPass;
			Ref<RenderPass> compositePass;
			


			Ref<RenderPass> activePass;
			std::vector<DrawCommand> drawList;
		};

		static SceneData* s_SceneData;
	};

}