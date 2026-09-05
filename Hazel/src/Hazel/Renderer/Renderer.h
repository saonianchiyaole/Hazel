#pragma once

#include "Hazel/Renderer/RenderCommand.h"


#include "Hazel/Renderer/Camera.h"

#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Resource/Resource.h"

#include "Hazel/Scene/Component.h"
#include "Hazel/Async/ThreadPool.h"

#include "Hazel/Renderer/RenderPass.h"
#include "Hazel/Renderer/Pipeline.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/RenderDevice.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <filesystem>
#include <mutex>
#include <queue>
#include <type_traits>
#include <vector>

namespace Hazel {
	
	class Application;
	class RenderPass;
	class Framebuffer;
	class Material;
	class Shader;
	class VertexArray;
	class Mesh;
	class UniformBufferSet;
	class UniformBuffer;
	class ThreadPool;
	class CommandQueue;
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

		static void StartRenderThread(Application* app);
		static void StopRenderThread();
		static bool IsOnRenderThread();

		static void Shutdown();

		static void BeginScene(const Camera& camera);
		static void BeginScene(const EditorCamera& camera);

		static void EndScene();

		static void SubmitVertex(const Ref<VertexArray>& vertexArray, Ref<Shader>& shader, const glm::mat4& transform = glm::mat4(1.0f));

		template<typename FuncT, typename ...Args>
		inline static auto SubmitTask(FuncT&& func, Args&&... args) -> std::future<std::invoke_result_t<FuncT, Args...>> {
			using ReturnType = std::invoke_result_t<FuncT, Args...>;

			if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
				std::promise<ReturnType> promise;
				std::future<ReturnType> result = promise.get_future();
				if constexpr (std::is_void_v<ReturnType>) {
					std::invoke(std::forward<FuncT>(func), std::forward<Args>(args)...);
					promise.set_value();
				}
				else {
					promise.set_value(std::invoke(std::forward<FuncT>(func), std::forward<Args>(args)...));
				}
				return result;
			}

			auto task = std::make_shared<std::packaged_task<ReturnType()>>(
				std::bind(std::forward<FuncT>(func), std::forward<Args>(args)...)
			);
			std::future<ReturnType> result = task->get_future();
			EnqueueRenderTask([task]() { (*task)(); });
			return result;
		}

		
		static void SetRenderDevice(Ref<RenderDevice> renderDevice);


		static Handle<VertexBuffer> CreateVertexBuffer(uint32_t size, std::vector<uint8_t> vertices = {});
		static Handle<IndexBuffer> CreateIndexBuffer(uint32_t size, std::vector<uint8_t> vertices = {});
		static Handle<UniformBuffer> CreateUniformBuffer(const std::string name);
		static Handle<UniformBufferSet> CreateUniformBufferSet(uint32_t amount, size_t size);
		static Handle<UniformBufferSet> CreateUniformBufferSet(size_t size);
		static Handle<VertexArray> CreateVertexArray();
		static Handle<CommandBuffer> CreateCommandBuffer();
		
		static Handle<Texture2D> CreateTexture2D(TextureInfo spec, std::vector<uint8_t> data);		
		static Handle<Texture2D> CreateTexture2D(std::filesystem::path path);
		/*static Handle<TextureCube> CreateTextureCube(std::vector<Ref<Texture2D>> textures);
		static Handle<TextureCube> CreateTextureCube(const std::string& path);
		static Handle<TextureCube> CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height);
		static Handle<TextureCube> CreateTextureCube();*/
		static Handle<Shader> CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		static Handle<Shader> CreateShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Handle<Shader> CreateShader(const std::string& filepath);
		static Handle<Material> CreateMaterial();
		static Handle<Material> CreateMaterial(std::filesystem::path filepath);
		static Handle<Material> CreateMaterial(const Handle<Shader>& shaderHandle);
		static Handle<Pipeline> CreatePipeline(const PipelineInfo& info);
		static Handle<Framebuffer> CreateFramebuffer(const FramebufferInfo& info, std::vector<Handle<Texture2D>> attachments = {});
		static Handle<RenderPass> CreateRenderPass(const RenderPassInfo& info);
		static Handle<Environment> CreateEnvironment(const std::string& filepath);

		template<typename T>
		static bool SetData(const std::string& name, const T& value, uint32_t index = 0) {
			if constexpr (std::is_same_v<std::remove_cv_t<T>, Handle<Texture2D>>) {
				return SetTexture(name, value, index);
			}

			HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
			Ref<RenderDevice> device = s_SceneData->renderDevice;
			T valueCopy = value;

			if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
				return device->SetData(name, valueCopy, index);
			}

			EnqueueRenderTask([device, name, valueCopy, index]() mutable {
				device->SetData(name, valueCopy, index);
			});
			return true;
		}

		static bool SetData(const std::string& name, const Handle<Texture2D>& texture, uint32_t index = 0) {
			return SetTexture(name, texture, index);
		}

		template<typename T>
		static bool SetData(const std::string& name, T* value, uint32_t index = 0) {
			if (!value) {
				HZ_CORE_ERROR("Renderer uniform data pointer with name {0} is null!", name);
				return false;
			}

			if constexpr (std::is_same_v<std::remove_cv_t<T>, Texture2D>) {
				HZ_CORE_ERROR("Texture uniform {0} cannot be set from Texture2D*. Use Handle<Texture2D> or Ref<Texture2D> instead.", name);
				return false;
			}
			else {
				HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
				Ref<RenderDevice> device = s_SceneData->renderDevice;
				std::remove_cv_t<T> valueCopy = *value;

				if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
					return device->SetData(name, &valueCopy, index);
				}

				EnqueueRenderTask([device, name, valueCopy, index]() mutable {
					device->SetData(name, &valueCopy, index);
				});
				return true;
			}
		}

		template<typename T>
		static bool SetData(const std::string& name, const Ref<T>& value, uint32_t index = 0) {
			if (!value) {
				HZ_CORE_ERROR("Renderer uniform ref with name {0} is null!", name);
				return false;
			}

			HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
			Ref<RenderDevice> device = s_SceneData->renderDevice;
			Ref<T> valueCopy = value;

			if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
				return device->SetData(name, valueCopy, index);
			}

			EnqueueRenderTask([device, name, valueCopy, index]() mutable {
				device->SetData(name, valueCopy, index);
			});
			return true;
		}

		static bool SetTexture(const std::string& name, const Handle<Texture2D>& texture, uint32_t index = 0);

		static void Init();
		static void BegineFrame();


		static void SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, int EntityHandle);
		static void SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, std::vector<Ref<Material>> materials, int EntityHandle);
		static void SubmitSubMesh(const Ref<Mesh> mesh, const Ref<SubMesh> subMesh, const TransformComponent& transformComponent, Ref<Material> material, int EntityHandle);


		static void SubmitLight(const LightComponent& lightComponent, const TransformComponent& transformComponent);
		static void SubmitSkybox(Ref<TextureCube> skyboxTextures);
		static void SubmitEnvironment(Ref<Environment> environment);

		static void BeginRenderPass(Handle<RenderPass> renderPass, bool clear = false);
		static void EndRenderPass();

		static void BindPipeline(Handle<Pipeline>);


		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }


		//RenderPass Function
		static void GeometryPass();
		static void CompositePass();

		static void FlushDrawList();
		static Handle<Framebuffer> GetGeometryPassFramebuffer();
		static Handle<Framebuffer> GetCompositePassFramebuffer();
		

		//Get/Set Data & Resources
		static void SetViewportSize(uint32_t width, uint32_t height);
		static void OnWindowResize(uint32_t width, uint32_t height);
		static Handle<Material> GetDefaultPBRMaterial();
		static Handle<Material> GetDefaultPhongMaterial();
		static Handle<Shader> GetDefaultPBRShader();
		static Handle<Texture2D> GetDefaultBlackQuadTexture();


		inline static uint32_t GetFrameInFlight() {
			return s_FrameInFlight;
		}
		static uint32_t GetCurrentFrameIndex();


		static uint8_t AllocateSlot();
		static uint8_t GetUsedTextureSlotAmount();
	private:
		
		struct SceneData {
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewProjectionMatrix;

			std::atomic<uint32_t> frameIndex = 0;


			Handle<Shader> defaultShader;
			Handle<Shader> skyboxShader;
			Handle<Shader> compositeShader;
			Handle<Shader> animationShader;
			Handle<Shader> defaultWhiteShader;


			Handle<Material> defaultPBRMaterial;
			Handle<Material> defaultPhongMaterial;
			Handle<Material> defaultAnimaitionMaterial;
			Handle<Material> skyboxMaterial;
			Handle<Material> defaultWhiteMaterial;

			Ref<Camera> primaryCamera;

			Handle<UniformBufferSet> cameraUniformBufferSet;
			Handle<UniformBufferSet> lightUniformBufferSet;
			Handle<VertexArray> skybox;
			Handle<Environment> environment;

			Handle<VertexArray> fullScreenQuad;
			
			
			const uint32_t maxTextureSlot = 32;
			uint32_t textureSlotIndex;

			Handle<RenderPass> geometryPass;
			Handle<RenderPass> compositePass;
			
			std::vector<Handle<Framebuffer>> framebuffers;

			Handle<Texture2D> blackQuadTexture;

			Handle<RenderPass> activePass;
			std::vector<DrawCommand> drawList;


			Ref<RenderDevice> renderDevice;
		};


		static SceneData* s_SceneData;

		static void RenderLoop(Application* app);
		static void EnqueueRenderTask(std::function<void()>&& task);

		static Scope<ThreadPool> s_AsyncThreadPool;
		static std::future<void> s_RenderLoopFuture;
		static std::atomic<bool> s_RenderLoopRunning;
		static std::atomic<uint64_t> s_RenderThreadToken;
		static uint32_t s_FrameInFlight;
		static Ref<CommandQueue> s_CommandQueue;

	};

}
