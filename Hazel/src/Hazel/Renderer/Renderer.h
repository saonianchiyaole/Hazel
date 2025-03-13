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
		Ref<SubMesh> subMesh;
		Ref<Material> material;
		glm::mat4 transform;
		int entityHandle;
	};

	// this should be Scene Renderer
	class Renderer {
	public:

		static void BeginScene(const Camera& camera);
		static void BeginScene(const EditorCamera& camera);

		static void EndScene();

		static void Submit(const Ref<VertexArray>& vertexArray, Ref<Shader>& shader, const glm::mat4& transform = glm::mat4(1.0f));

		static void Init();

		static void SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, int EntityHandle);
		static void SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, std::vector<Ref<Material>> materials, int EntityHandle);
		static void SubmitSubMesh(const Ref<Mesh> mesh, const Ref<SubMesh> subMesh, const TransformComponent& transformComponent, Ref<Material> material, int EntityHandle);


		static void SubmitLight(const LightComponent& lightComponent, const TransformComponent& transformComponent);
		static void SubmitSkybox(Ref<TextureCube> skyboxTextures);
		static void SubmitEnvironment(Ref<Environment> environment);

		static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = false);
		static void EndRenderPass();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }


		//RenderPass Function
		static void GeometryPass();
		static void CompositePass();

		static void FlushDrawList();
		static Ref<Framebuffer> GetGeometryPassFramebuffer();
		static Ref<Framebuffer> GetCompositePassFramebuffer();
		

		//Get/Set Data & Resources
		static void SetViewportSize(uint32_t width, uint32_t height);
		static void OnWindowResize(uint32_t width, uint32_t height);
		static Ref<Material> GetDefaultPBRMaterial();
		static Ref<Material> GetDefaultPhongMaterial();
		static Ref<Shader> GetDefaultPBRShader();
		static Ref<Texture2D> GetDefaultBlackQuadTexture();


		static uint8_t AllocateSlot();
		static uint8_t GetUsedTextureSlotAmount();
	private:
		
		struct SceneData {
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewProjectionMatrix;

			Ref<Shader> defaultShader;
			Ref<Shader> skyboxShader;
			Ref<Shader> compositeShader;
			Ref<Shader> animationShader;
			Ref<Shader> defaultWhiteShader;


			Ref<Material> defaultPBRMaterial;
			Ref<Material> defaultPhongMaterial;
			Ref<Material> defaultAnimaitionMaterial;
			Ref<Material> skyboxMaterial;
			Ref<Material> defaultWhiteMaterial;

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
			
			Ref<Texture2D> blackQuadTexture;

			Ref<RenderPass> activePass;
			std::vector<DrawCommand> drawList;
		};

		static SceneData* s_SceneData;
	};

}