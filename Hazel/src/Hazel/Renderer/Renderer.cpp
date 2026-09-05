#include "hzpch.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/RenderCommand.h"


#include "Hazel/Async/Thread.h"
#include "Hazel/Core/Application.h"
#include "Hazel/Core/CommandQueue.h"
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Environment.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Swapchain.h"
#include "Hazel/Renderer/Pipeline.h"
#include "Hazel/Renderer/RenderPass.h"
#include "Hazel/Renderer/RenderDevice.h"
#include "Hazel/Renderer/Mesh.h"
#include "Hazel/Renderer/Buffer.h"

#include <chrono>

namespace Hazel {

	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;
	uint32_t Renderer::s_FrameInFlight = 3;
	Scope<ThreadPool> Renderer::s_AsyncThreadPool = nullptr;
	std::future<void> Renderer::s_RenderLoopFuture;
	std::atomic<bool> Renderer::s_RenderLoopRunning = false;
	std::atomic<uint64_t> Renderer::s_RenderThreadToken = 0;


	Ref<CommandQueue> Renderer::s_CommandQueue = nullptr;

	float skyboxVertices[] = {
		// Positions         
		-1.0f, -1.0f, -1.0f,  // 0
		 1.0f, -1.0f, -1.0f,  // 1
		 1.0f,  1.0f, -1.0f,  // 2
		-1.0f,  1.0f, -1.0f,  // 3
		-1.0f, -1.0f,  1.0f,  // 4
		 1.0f, -1.0f,  1.0f,  // 5
		 1.0f,  1.0f,  1.0f,  // 6
		-1.0f,  1.0f,  1.0f   // 7
	};

	uint32_t skyboxIndices[] = {
		// Front face
		0, 2, 1,
		0, 3, 2,

		// Back face
		4, 6, 5,
		4, 7, 6,

		// Left face
		0, 7, 3,
		0, 4, 7,

		// Right face
		1, 6, 2,
		1, 5, 6,

		// Bottom face
		0, 5, 1,
		0, 4, 5,

		// Top face
		3, 6, 2,
		3, 7, 6
	};

	float fullScreenQuadVertices[] = {
		// Vertices         // TexCoord
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 0 (左下角)
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 1 (右下角)
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 2 (左上角)
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f  // 3 (右上角)
	};
	uint32_t fullScreenQuadIndices[] = {
		0, 1, 2, // 第一个三角形 (左下角 -> 右下角 -> 左上角)
		1, 3, 2  // 第二个三角形 (右下角 -> 右上角 -> 左上角)
	};


	namespace {
		uint64_t GetCurrentThreadToken() {
			return static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
		}
	}

	bool Renderer::IsOnRenderThread()
	{
		const uint64_t renderToken = s_RenderThreadToken.load();
		return renderToken != 0 && renderToken == GetCurrentThreadToken();
	}

	void Renderer::SetRenderDevice(Ref<RenderDevice> renderDevice)
	{
		s_SceneData->renderDevice = renderDevice;
	}

	Handle<VertexBuffer> Renderer::CreateVertexBuffer(uint32_t size, std::vector<uint8_t> vertices)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<VertexBuffer> handle = ResourceManager<VertexBuffer>::Create(size);
		
		std::vector<uint8_t> copy = vertices;

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateVertexBuffer(handle, size, std::move(copy));
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}

			s_CommandQueue->push(device.get(), (&RenderDevice::CreateIndexBuffer), handle, size, std::move(copy));
		}

		return handle;
	}

	Handle<IndexBuffer> Renderer::CreateIndexBuffer(uint32_t size, std::vector<uint8_t> vertices)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<IndexBuffer> handle = ResourceManager<IndexBuffer>::Create(size);

		std::vector<uint8_t> copy = vertices;

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateIndexBuffer(handle, size, std::move(copy));
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}

			s_CommandQueue->push(device.get(), (&RenderDevice::CreateIndexBuffer), handle, size, std::move(copy));
		}

		return handle;
	}

	Handle<UniformBuffer> Renderer::CreateUniformBuffer(std::string name)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<UniformBuffer> handle = ResourceManager<UniformBuffer>::Create();

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateUniformBuffer(handle, name);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<UniformBuffer>, const std::string)>(&RenderDevice::CreateUniformBuffer), handle, name);
		}

		return handle;
	}
	Handle<UniformBufferSet> Renderer::CreateUniformBufferSet(uint32_t amount, size_t size)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<UniformBufferSet> handle = ResourceManager<UniformBufferSet>::Create(amount, size);

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateUniformBufferSet(handle, amount, size);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<UniformBufferSet>, uint32_t, size_t)>(&RenderDevice::CreateUniformBufferSet), handle, amount, size);
		}

		return handle;
	}

	Handle<UniformBufferSet> Renderer::CreateUniformBufferSet(size_t size)
	{
		return CreateUniformBufferSet(GetFrameInFlight(), size);
	}

	Handle<VertexArray> Renderer::CreateVertexArray()
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<VertexArray> handle = ResourceManager<VertexArray>::Create();

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateVertexArray(handle);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<VertexArray>)>(&RenderDevice::CreateVertexArray), handle);
		}

		return handle;
	}

	Handle<CommandBuffer> Renderer::CreateCommandBuffer()
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<CommandBuffer> handle = ResourceManager<CommandBuffer>::Create();

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateCommandBuffer(handle);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<CommandBuffer>)>(&RenderDevice::CreateCommandBuffer), handle);
		}

		return handle;
	}

	Handle<Texture2D> Renderer::CreateTexture2D(TextureInfo spec, std::vector<uint8_t> data)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<Texture2D> handle = ResourceManager<Texture2D>::Create(spec);

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateTexture2D(handle, spec, std::move(data));
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<Texture2D>, TextureInfo, std::vector<uint8_t>)>(&RenderDevice::CreateTexture2D), handle, spec, std::move(data));
		}

		return handle;
	}

	Handle<Texture2D> Renderer::CreateTexture2D(std::filesystem::path path)
	{
		TextureInfo textureInfo;
		std::vector<uint8_t> data;
		if (!Utils::LoadTextureDataFromFile(path, textureInfo, data)) {
			return Handle<Texture2D>();
		}

		return CreateTexture2D(textureInfo, std::move(data));
	}

	/*Handle<TextureCube> Renderer::CreateTextureCube(std::vector<Ref<Texture2D>> textures)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<TextureCube> handle = device->AllocateHandle<TextureCube>();

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateTextureCube(handle, textures);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<TextureCube>, std::vector<Ref<Texture2D>>)>(&RenderDevice::CreateTextureCube), handle, std::move(textures));
		}

		return handle;
	}

	Handle<TextureCube> Renderer::CreateTextureCube(const std::string& path)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<TextureCube> handle = device->AllocateHandle<TextureCube>();

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateTextureCube(handle, path);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<TextureCube>, const std::string&)>(&RenderDevice::CreateTextureCube), handle, path);
		}

		return handle;
	}

	Handle<TextureCube> Renderer::CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<TextureCube> handle = device->AllocateHandle<TextureCube>();

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateTextureCube(handle, format, width, height);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<TextureCube>, TextureFormat, uint32_t, uint32_t)>(&RenderDevice::CreateTextureCube), handle, format, width, height);
		}

		return handle;
	}

	Handle<TextureCube> Renderer::CreateTextureCube()
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<TextureCube> handle = device->AllocateHandle<TextureCube>();

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateTextureCube(handle);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<TextureCube>)>(&RenderDevice::CreateTextureCube), handle);
		}

		return handle;
	}*/

	Handle<Shader> Renderer::CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<Shader> handle = ResourceManager<Shader>::Create();

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateShader(handle, vertexSrc, fragmentSrc);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<Shader>, const std::string&, const std::string&)>(&RenderDevice::CreateShader), handle, vertexSrc, fragmentSrc);
		}

		return handle;
	}

	Handle<Shader> Renderer::CreateShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<Shader> handle = ResourceManager<Shader>::Create(name, vertexSrc, fragmentSrc);

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateShader(handle, name, vertexSrc, fragmentSrc);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<Shader>, const std::string&, const std::string&, const std::string&)>(&RenderDevice::CreateShader), handle, name, vertexSrc, fragmentSrc);
		}

		return handle;
	}

	Handle<Shader> Renderer::CreateShader(const std::string& filepath)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<Shader> handle = ResourceManager<Shader>::Create(filepath);

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateShader(handle, filepath);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<Shader>, const std::string&)>(&RenderDevice::CreateShader), handle, filepath);
		}

		return handle;
	}

	Handle<Material> Renderer::CreateMaterial()
	{
		return ResourceManager<Material>::Create(Material::Create());
	}

	Handle<Material> Renderer::CreateMaterial(std::filesystem::path filepath)
	{
		return ResourceManager<Material>::Create(Material::Create(filepath));
	}

	Handle<Material> Renderer::CreateMaterial(const Handle<Shader>& shaderHandle)
	{
		Ref<Shader> shader = ResourceManager<Shader>::Get(shaderHandle);
		return ResourceManager<Material>::Create(MakeRef<Material>(shader));

	}

	Handle<Pipeline> Renderer::CreatePipeline(const PipelineInfo& info)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<Pipeline> handle = ResourceManager<Pipeline>::Create(info);
		return handle;
	}

	Handle<Framebuffer> Renderer::CreateFramebuffer(const FramebufferInfo& info, std::vector<Handle<Texture2D>> attachments)
	{

		HZ_CORE_ASSERT(info.attachments.size(), "Framebuffer must have attachment");
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<Framebuffer> handle = ResourceManager<Framebuffer>::Create(info);

		if (attachments.size() == 0) {



		}
			


		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateFramebuffer(handle, info, attachments);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), &RenderDevice::CreateFramebuffer, handle, info, std::move(attachments));
		}

		return handle;
	}

	Handle<RenderPass> Renderer::CreateRenderPass(const RenderPassInfo& info)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<RenderPass> handle = ResourceManager<RenderPass>::Create(info);
		return handle;
	}
	
	

	Handle<Environment> Renderer::CreateEnvironment(const std::string& filepath)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;
		Handle<Environment> handle = ResourceManager<Environment>::Create(filepath);

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->CreateEnvironment(handle, filepath);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<Environment>, const std::string&)>(&RenderDevice::CreateEnvironment), handle, filepath);
		}

		return handle;
	}

	void Renderer::EnqueueRenderTask(std::function<void()>&& task)
	{
		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			task();
			return;
		}

		if (!s_CommandQueue) {
			s_CommandQueue = MakeRef<CommandQueue>();
		}

		struct TaskInvoker {
			void Invoke(std::function<void()> fn) {
				fn();
			}
		};
		static TaskInvoker s_TaskInvoker;

		s_CommandQueue->push(&s_TaskInvoker, &TaskInvoker::Invoke, std::move(task));
	}

	void Renderer::RenderLoop(Application* app)
	{
		Utils::SetCurrentThreadName("RenderLoop");
		s_RenderThreadToken.store(GetCurrentThreadToken());

		while (s_RenderLoopRunning.load() && app && app->IsRunning()) {
			if (s_CommandQueue && app->GetRenderThreadFrameCount() >= app->GetMainThreadFrameCount()) {
				s_CommandQueue->wait_for_pending(1);
			}

			if (s_CommandQueue) {
				s_CommandQueue->flush_if_pending();
			}

			if (app->GetRenderThreadFrameCount() < app->GetMainThreadFrameCount()) {
				app->NextRenderFrame();
			}
		}

		if (s_CommandQueue) {
			s_CommandQueue->flush_all();
		}
		s_RenderThreadToken.store(0);
	}

	void Renderer::Init() {

		RenderCommand::Init();
		if (!s_CommandQueue) {
			s_CommandQueue = MakeRef<CommandQueue>();
		}

		//s_SceneData->textureSlotIndex = 0;

		//uint32_t frameInFlight = GetFrameInFlight();

		//s_SceneData->cameraUniformBufferSet = UniformBufferSet::Create(sizeof(CameraUniformBuffer));
		//s_SceneData->lightUniformBufferSet = UniformBufferSet::Create(sizeof(LightUniformBuffer));

		//ShaderLibrary::Load("assets/Shaders/Standard.glsl");
		//s_SceneData->defaultShader = ShaderLibrary::Get("Standard");

		//ShaderLibrary::Load("assets/Shaders/Skybox.glsl");
		//s_SceneData->skyboxShader = ShaderLibrary::Get("Skybox");
		//
		//ShaderLibrary::Load("assets/Shaders/simple.glsl");
		//s_SceneData->defaultWhiteShader = ShaderLibrary::Get("simple");


		//// Initialize shader, material
		//{
		//	s_SceneData->skybox = VertexArray::Create();
		//	BufferLayout skyboxBufferLayout = std::vector<Hazel::BufferElement>{
		//		{Hazel::ShaderDataType::Float3, "a_Position" }
		//	};
		//	Ref<VertexBuffer> skyboxVertexBuffer = VertexBuffer::Create(skyboxVertices, sizeof(skyboxVertices));
		//	skyboxVertexBuffer->SetLayout(skyboxBufferLayout);
		//	s_SceneData->skybox->AddVertexBuffer(skyboxVertexBuffer);

		//	Ref<IndexBuffer> skyboxIndexBuffer = IndexBuffer::Create(skyboxIndices, sizeof(skyboxIndices) / sizeof(uint32_t));
		//	s_SceneData->skybox->SetIndexBuffer(skyboxIndexBuffer);

		//	s_SceneData->defaultPBRMaterial = Material::Create("assets/Material/Standard.material");
		//	s_SceneData->defaultWhiteMaterial = Material::Create();
		//	s_SceneData->defaultWhiteMaterial->SetShader(s_SceneData->defaultWhiteShader);


		//	s_SceneData->skyboxMaterial = Material::Create();
		//	s_SceneData->skyboxMaterial->SetShader(s_SceneData->skyboxShader);

		//}
		//
		////Set Dufault Light Uniform


		////Set FullScreenQuad 
		//{
		//	s_SceneData->fullScreenQuad =
		//		s_SceneData->fullScreenQuad = VertexArray::Create();
		//	BufferLayout fullScreenQuadBufferLayout = std::vector<Hazel::BufferElement>{
		//		{Hazel::ShaderDataType::Float3, "a_Position" },
		//		{Hazel::ShaderDataType::Float2, "a_TexCoord" }

		//	};
		//	Ref<VertexBuffer> fullScreenQuadVertexBuffer = VertexBuffer::Create(fullScreenQuadVertices, sizeof(fullScreenQuadVertices));
		//	fullScreenQuadVertexBuffer->SetLayout(fullScreenQuadBufferLayout);
		//	s_SceneData->fullScreenQuad->AddVertexBuffer(fullScreenQuadVertexBuffer);

		//	Ref<IndexBuffer> fullScreenQuadIndexBuffer = IndexBuffer::Create(fullScreenQuadIndices, sizeof(fullScreenQuadIndices) / sizeof(uint32_t));
		//	s_SceneData->fullScreenQuad->SetIndexBuffer(fullScreenQuadIndexBuffer);
		//}


		//// default texture
		//{
		//	uint32_t blackTextureData = 0;
		//	s_SceneData->blackQuadTexture = Texture2D::Create(1, 1);
		//	s_SceneData->blackQuadTexture->SetData(&blackTextureData, sizeof(uint32_t));

		//}

		Renderer2D::Init();


		//set geometry pass
		{
			/*FramebufferSpecification geometryFramebufferSpec;
			geometryFramebufferSpec.width = Hazel::Application::GetInstance().GetWindow().GetWidth();
			geometryFramebufferSpec.height = Hazel::Application::GetInstance().GetWindow().GetHeight();
			geometryFramebufferSpec.attachments = { PixelFormat::RGBA, PixelFormat::R, PixelFormat::Depth };
			geometryFramebufferSpec.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			Ref<Framebuffer> geometryTargetFramebuffer = Framebuffer::Create(geometryFramebufferSpec);

			PipelineSpecification geometryPipelineSpc;
			geometryPipelineSpc.shader = ;
			geometryPipelineSpc.bufferLayout = ;
			geometryPipelineSpc.targetFramebuffer;
			Ref<Pipeline> geometryPipeline = Pipeline::Create(geometryPipelineSpc);

			RenderPassSpecification geometryPassSpec;
			geometryPassSpec.pipeline = geometryPipeline;
			geometryPassSpec.targetFramebuffer = geometryTargetFramebuffer;
			s_SceneData->geometryPass = RenderPass::Create(geometryPassSpec);*/
		}

		//set composite pass
		/*{
			FramebufferSpecification compositeFramebufferSpec;
			compositeFramebufferSpec.width = Hazel::Application::GetInstance().GetWindow().GetWidth();
			compositeFramebufferSpec.height = Hazel::Application::GetInstance().GetWindow().GetHeight();
			compositeFramebufferSpec.attachments = { PixelFormat::RGBA, PixelFormat::R};
			Ref<Framebuffer> compositeTargetFramebuffer = Framebuffer::Create(compositeFramebufferSpec);
			RenderPassSpecification compositePassSpec;
			compositePassSpec.targetFramebuffer = compositeTargetFramebuffer;
			s_SceneData->compositePass = RenderPass::Create(compositePassSpec);

			s_SceneData->compositeShader = ShaderLibrary::Load("assets/Shaders/Composite.glsl");

		}*/



	}

	void Renderer::BegineFrame()
	{
		s_SceneData->frameIndex = Application::GetInstance().GetWindow().GetSwapchain()->GetCurrentFrameIndex();
		RenderCommand::BeginFrame();
		Renderer2D::BeginFrame();
	}

	void Renderer::StartRenderThread(Application* app)
	{
		if (s_RenderLoopRunning.load()) {
			return;
		}

		if (!s_AsyncThreadPool) {
			s_AsyncThreadPool = MakeScope<ThreadPool>();
		}

		if (!s_AsyncThreadPool->IsRunning()) {
			const uint32_t hardwareCount = std::thread::hardware_concurrency();
			const uint32_t workerCount = hardwareCount > 1 ? hardwareCount : 2;
			s_AsyncThreadPool->Start(workerCount, "AsyncWorker");
		}

		s_RenderLoopRunning.store(true);
		s_RenderLoopFuture = s_AsyncThreadPool->Enqueue([app]() {
			Renderer::RenderLoop(app);
			});
	}

	void Renderer::StopRenderThread()
	{
		if (!s_RenderLoopRunning.load()) {
			return;
		}

		s_RenderLoopRunning.store(false);
		s_RenderThreadToken.store(0);

		if (s_RenderLoopFuture.valid()) {
			s_RenderLoopFuture.get();
		}

		if (s_AsyncThreadPool && s_AsyncThreadPool->IsRunning()) {
			s_AsyncThreadPool->Stop();
		}
	}


	void Renderer::Shutdown() {

		StopRenderThread();
		RenderCommand::Shutdown();

	}

	void Renderer::BeginScene(const Camera& camera)
	{
		/*s_SceneData->ViewMatrix = camera.GetViewMatrix();
		s_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();


		CameraUniformBuffer cameraUniformBufferData;
		cameraUniformBufferData.viewProjectionMatrix = camera.GetViewProjectionMatrix();
		cameraUniformBufferData.position = { camera.GetPosition(), 0.0f };

		s_SceneData->cameraUniformBufferSet->Get()->SetData(&cameraUniformBufferData, sizeof(CameraUniformBuffer), 0);*/

	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		/*s_SceneData->ViewMatrix = camera.GetViewMatrix();
		s_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();

		CameraUniformBuffer cameraUniformBufferData;
		cameraUniformBufferData.viewProjectionMatrix = camera.GetViewProjectionMatrix();
		cameraUniformBufferData.position = { camera.GetPosition(), 0.0f };

		s_SceneData->cameraUniformBufferSet->Get()->SetData(&cameraUniformBufferData, sizeof(CameraUniformBuffer), 0);*/
	}

	void Renderer::EndScene()
	{

		/*FlushDrawList();

		s_SceneData->environment = nullptr;
		s_SceneData->textureSlotIndex = 0;
		s_SceneData->drawList.clear();*/
	}
	void Renderer::SubmitVertex(const Ref<VertexArray>& vertexArray, Ref<Shader>& shader, const glm::mat4& transform)
	{

		// todo : change to renderpass input
		/*std::dynamic_pointer_cast<OpenGLShader>(shader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_View", s_SceneData->ViewMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Projection", s_SceneData->ProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);*/

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}



	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, int EntityHandle)
	{
		//shader->Bind();
		//shader->SetMat4("u_Transform", transformComponent.transform);
		//mesh->m_VertexArray->Bind();

		/*for (Ref<SubMesh> subMesh : mesh->GetSubMeshes()) {
			s_SceneData->drawList.push_back({ mesh, subMesh, s_SceneData->defaultPBRMaterial, transformComponent.transform, EntityHandle });
		}*/

		//RenderCommand::DrawIndexed(mesh->m_VertexArray);
	}

	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, std::vector<Ref<Material>> materials, int entityHandle)
	{

		/*for (Ref<SubMesh> subMesh : mesh->GetSubMeshes()) {
			SubmitSubMesh(mesh, subMesh, transformComponent.transform,
				subMesh->materialIndex < materials.size() ? materials[subMesh->materialIndex] : s_SceneData->defaultWhiteMaterial
				, entityHandle);
		}

		s_SceneData->textureSlotIndex = 0;*/

	}

	void Renderer::SubmitSubMesh(const Ref<Mesh> mesh, const Ref<SubMesh> subMesh, const TransformComponent& transformComponent, Ref<Material> material, int entityHandle)
	{
		s_SceneData->drawList.push_back({ mesh, subMesh, material, transformComponent.transform, entityHandle });
	}



	void Renderer::SubmitLight(const LightComponent& lightComponent, const TransformComponent& transformComponent)
	{

		/*uint32_t frameIndex = GetCurrentFrameIndex();

		LightUniformBuffer lightUniformBufferData;
		lightUniformBufferData.position = { transformComponent.translate, 0.0f };

		glm::quat quaternion = glm::quat(transformComponent.rotation);
		glm::mat3 rotationMatrix = glm::toMat3(quaternion);

		lightUniformBufferData.direction = { glm::normalize(rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f)), 0.0f };
		lightUniformBufferData.color = { lightComponent.color, 0.0f };

		s_SceneData->lightUniformBufferSet->Get(frameIndex)->SetData(&lightUniformBufferData, sizeof(LightUniformBuffer), 0);*/
	}

	void Renderer::SubmitSkybox(Ref<TextureCube> skyboxTextures)
	{
		//s_SceneData->skybox->Bind();
		//s_SceneData->skyboxShader->Bind();
		//s_SceneData->skyboxShader->SetInt("u_SkyBox", 0);
		//s_SceneData->skyboxShader->SetMat4("u_View", s_SceneData->ViewMatrix);
		//s_SceneData->skyboxShader->SetMat4("u_Projection", s_SceneData->ProjectionMatrix);


		////s_SceneData->skyboxMayerial->SetData<int>("u_SkyBox", 0);
		////s_SceneData->skyboxMayerial->SetData("u_View", s_SceneData->ViewMatrix);
		////s_SceneData->skyboxMayerial->SetData("u_Projection", s_SceneData->ProjectionMatrix);
		////s_SceneData->skyboxMayerial->SetData();
		//skyboxTextures->Bind();

		//RenderCommand::DrawIndexed(s_SceneData->skybox);
	}

	void Renderer::SubmitEnvironment(Ref<Environment> environment)
	{

		//s_SceneData->environment = environment;
	}

	void Renderer::BeginRenderPass(Handle<RenderPass> renderPass, bool clear)
	{
		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");

		Ref<RenderDevice> device = s_SceneData->renderDevice;
		s_SceneData->activePass = renderPass;

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			device->BeginRenderPass(renderPass);
		}
		else {
			if (!s_CommandQueue) {
				s_CommandQueue = MakeRef<CommandQueue>();
			}
			s_CommandQueue->push(device.get(), static_cast<void (RenderDevice::*)(Handle<RenderPass>)>(&RenderDevice::BeginRenderPass), renderPass);
		}
	}

	void Renderer::EndRenderPass()
	{
		if (!RenderDevice::Get().HasResource(s_SceneData->activePass)) {
			s_SceneData->activePass = Handle<RenderPass>();
			return;
		}

		Ref<RenderPass> activePass = RenderDevice::Get().GetResource(s_SceneData->activePass);
		activePass->SetFramebuffer(Handle<Framebuffer>());
		s_SceneData->activePass = Handle<RenderPass>();
	}


	bool Renderer::SetTexture(const std::string& name, const Handle<Texture2D>& texture, uint32_t index) {

		HZ_CORE_ASSERT(s_SceneData->renderDevice, "RenderDevice is not set");
		Ref<RenderDevice> device = s_SceneData->renderDevice;

		if (IsOnRenderThread() || !s_RenderLoopRunning.load()) {
			return device->SetTexture(name, texture, index);
		}

		EnqueueRenderTask([device, name, texture, index]() {
			device->SetTexture(name, texture, index);
		});
		return true;
	}


	void Renderer::GeometryPass()
	{
		
	}


	void Renderer::CompositePass()
	{
		

	}

	void Renderer::FlushDrawList()
	{
		
	}

	Handle<Framebuffer> Renderer::GetGeometryPassFramebuffer()
	{
		// todo
		return Handle<Framebuffer>();
	}

	Handle<Framebuffer> Renderer::GetCompositePassFramebuffer()
	{
		// todo
		return Handle<Framebuffer>();
	}


	void Renderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		//s_SceneData->compositePass->GetInfo().targetFramebuffer->Resize(glm::vec2{ width, height });
		//s_SceneData->compositePass->GetInfo().targetFramebuffer->Resize(glm::vec2{ width, height });
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{

		EnqueueRenderTask([width, height]() {
			RenderCommand::SetViewPort(0, 0, width, height);
			});

		Renderer2D::Resize(glm::vec2(width, height));

	}

	Handle<Material> Renderer::GetDefaultPBRMaterial()
	{
		return s_SceneData->defaultPBRMaterial;
	}
	Handle<Material> Renderer::GetDefaultPhongMaterial()
	{
		return s_SceneData->defaultPhongMaterial;
	}
	Handle<Shader> Renderer::GetDefaultPBRShader()
	{
		return  s_SceneData->defaultShader;
	}

	Handle<Texture2D> Renderer::GetDefaultBlackQuadTexture()
	{
		return s_SceneData->blackQuadTexture;
	}

	uint32_t Renderer::GetCurrentFrameIndex() {
		return s_SceneData->frameIndex;
	}

	uint8_t Renderer::AllocateSlot()
	{
		uint8_t slot = s_SceneData->textureSlotIndex;
		s_SceneData->textureSlotIndex++;
		return slot;
	}

	uint8_t Renderer::GetUsedTextureSlotAmount()
	{
		return s_SceneData->textureSlotIndex - 1;
	}
}
