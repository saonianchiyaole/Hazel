#include "hzpch.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Hazel/Core/Application.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Scene/Component.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Environment.h"
#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel {

	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;


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


	void Renderer::Init() {
		RenderCommand::Init();
		Renderer2D::Init();

		s_SceneData->textureSlotIndex = 0;

		s_SceneData->cameraUniformBuffer = UniformBuffer::Create(sizeof(CameraUniformBuffer), 0);
		s_SceneData->lightUniformBuffer = UniformBuffer::Create(sizeof(LightUniformBuffer), 1);

		ShaderLibrary::Load("assets/Shaders/Standard.glsl");
		s_SceneData->defaultShader = ShaderLibrary::Get("Standard");

		ShaderLibrary::Load("assets/Shaders/Skybox.glsl");
		s_SceneData->skyboxShader = ShaderLibrary::Get("Skybox");
		
		ShaderLibrary::Load("assets/Shaders/simple.glsl");
		s_SceneData->defaultWhiteShader = ShaderLibrary::Get("simple");

		//ShaderLibrary::Load("assets/Shaders/Animation.glsl");
		//s_SceneData->animationShader = ShaderLibrary::Get("Animation");

		{
			s_SceneData->skybox = VertexArray::Create();
			BufferLayout skyboxBufferLayout = std::vector<Hazel::BufferElement>{
				{Hazel::ShaderDataType::Float3, "a_Position" }
			};
			Ref<VertexBuffer> skyboxVertexBuffer = VertexBuffer::Create(skyboxVertices, sizeof(skyboxVertices));
			skyboxVertexBuffer->SetLayout(skyboxBufferLayout);
			s_SceneData->skybox->AddVertexBuffer(skyboxVertexBuffer);

			Ref<IndexBuffer> skyboxIndexBuffer = IndexBuffer::Create(skyboxIndices, sizeof(skyboxIndices) / sizeof(uint32_t));
			s_SceneData->skybox->SetIndexBuffer(skyboxIndexBuffer);

			s_SceneData->defaultPBRMaterial = Material::Create("assets/Material/Standard.material");
			s_SceneData->defaultWhiteMaterial = Material::Create();
			s_SceneData->defaultWhiteMaterial->SetShader(s_SceneData->defaultWhiteShader);


			s_SceneData->skyboxMaterial = Material::Create();
			s_SceneData->skyboxMaterial->SetShader(s_SceneData->skyboxShader);

		}
		//Set Dufault Light Uniform


		//Set FullScreenQuad 
		{
			s_SceneData->fullScreenQuad =
				s_SceneData->fullScreenQuad = VertexArray::Create();
			BufferLayout fullScreenQuadBufferLayout = std::vector<Hazel::BufferElement>{
				{Hazel::ShaderDataType::Float3, "a_Position" },
				{Hazel::ShaderDataType::Float2, "a_TexCoord" }

			};
			Ref<VertexBuffer> fullScreenQuadVertexBuffer = VertexBuffer::Create(fullScreenQuadVertices, sizeof(fullScreenQuadVertices));
			fullScreenQuadVertexBuffer->SetLayout(fullScreenQuadBufferLayout);
			s_SceneData->fullScreenQuad->AddVertexBuffer(fullScreenQuadVertexBuffer);

			Ref<IndexBuffer> fullScreenQuadIndexBuffer = IndexBuffer::Create(fullScreenQuadIndices, sizeof(fullScreenQuadIndices) / sizeof(uint32_t));
			s_SceneData->fullScreenQuad->SetIndexBuffer(fullScreenQuadIndexBuffer);
		}


		//set geometry pass
		{
			FramebufferSpecification geometryFramebufferSpec;
			geometryFramebufferSpec.width = Hazel::Application::GetInstance().GetWindow().GetWidth();
			geometryFramebufferSpec.height = Hazel::Application::GetInstance().GetWindow().GetHeight();
			geometryFramebufferSpec.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
			geometryFramebufferSpec.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			Ref<Framebuffer> geometryTargetFramebuffer = Framebuffer::Create(geometryFramebufferSpec);
			RenderPassSpec geometryPassSpec;
			geometryPassSpec.targetFrameBuffer = geometryTargetFramebuffer;
			s_SceneData->geometryPass = RenderPass::Create(geometryPassSpec);
		}

		//set composite pass
		{
			FramebufferSpecification compositeFramebufferSpec;
			compositeFramebufferSpec.width = Hazel::Application::GetInstance().GetWindow().GetWidth();
			compositeFramebufferSpec.height = Hazel::Application::GetInstance().GetWindow().GetHeight();
			compositeFramebufferSpec.attachments = { FramebufferTextureFormat::RGBA8 };
			Ref<Framebuffer> compositeTargetFramebuffer = Framebuffer::Create(compositeFramebufferSpec);
			RenderPassSpec compositePassSpec;
			compositePassSpec.targetFrameBuffer = compositeTargetFramebuffer;
			s_SceneData->compositePass = RenderPass::Create(compositePassSpec);

			s_SceneData->compositeShader = ShaderLibrary::Load("assets/Shaders/Composite.glsl");

		}

	}

	void Renderer::BeginScene(const Camera& camera)
	{
		s_SceneData->ViewMatrix = camera.GetViewMatrix();
		s_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();


		CameraUniformBuffer cameraUniformBufferData;
		cameraUniformBufferData.viewProjectionMatrix = camera.GetViewProjectionMatrix();
		cameraUniformBufferData.position = { camera.GetPosition(), 0.0f };

		s_SceneData->cameraUniformBuffer->SetData(&cameraUniformBufferData, sizeof(CameraUniformBuffer), 0);

	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		s_SceneData->ViewMatrix = camera.GetViewMatrix();
		s_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();

		CameraUniformBuffer cameraUniformBufferData;
		cameraUniformBufferData.viewProjectionMatrix = camera.GetViewProjectionMatrix();
		cameraUniformBufferData.position = { camera.GetPosition(), 0.0f };

		s_SceneData->cameraUniformBuffer->SetData(&cameraUniformBufferData, sizeof(CameraUniformBuffer), 0);
	}

	void Renderer::EndScene()
	{

		FlushDrawList();

		s_SceneData->environment = nullptr;
		s_SceneData->textureSlotIndex = 0;
		s_SceneData->drawList.clear();
	}
	void Renderer::Submit(const Ref<VertexArray>& vertexArray, Ref<Shader>& shader, const glm::mat4& transform)
	{

		std::dynamic_pointer_cast<OpenGLShader>(shader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_View", s_SceneData->ViewMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Projection", s_SceneData->ProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}



	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, Ref<Shader> shader, UUID entityID)
	{
		//shader->Bind();
		//shader->SetMat4("u_Transform", transformComponent.transform);
		//mesh->m_VertexArray->Bind();

		for (Ref<SubMesh> subMesh : mesh->GetSubMeshes()) {
			s_SceneData->drawList.push_back({ mesh, subMesh, s_SceneData->defaultPBRMaterial, transformComponent.transform });
		}

		//RenderCommand::DrawIndexed(mesh->m_VertexArray);
	}

	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, std::vector<Ref<Material>> materials, int entityHandle)
	{

		for (Ref<SubMesh> subMesh : mesh->GetSubMeshes()) {
			SubmitSubMesh(mesh, subMesh, transformComponent.transform, 
				subMesh->materialIndex < materials.size() ? materials[subMesh->materialIndex] : s_SceneData->defaultWhiteMaterial
				, entityHandle);
		}

		s_SceneData->textureSlotIndex = 0;

	}

	void Renderer::SubmitSubMesh(const Ref<Mesh> mesh, const Ref<SubMesh> subMesh, const TransformComponent& transformComponent, Ref<Material> material, int entityHandle)
	{
		s_SceneData->drawList.push_back({ mesh, subMesh, material, transformComponent.transform, entityHandle });
	}



	void Renderer::SubmitLight(const LightComponent& lightComponent, const TransformComponent& transformComponent)
	{
		LightUniformBuffer lightUniformBufferData;
		lightUniformBufferData.position = { transformComponent.translate, 0.0f };

		glm::quat quaternion = glm::quat(transformComponent.rotation);
		glm::mat3 rotationMatrix = glm::toMat3(quaternion);

		lightUniformBufferData.direction = { glm::normalize(rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f)), 0.0f };
		lightUniformBufferData.color = { lightComponent.color, 0.0f };

		s_SceneData->lightUniformBuffer->SetData(&lightUniformBufferData, sizeof(LightUniformBuffer), 0);
	}

	void Renderer::SubmitSkybox(Ref<TextureCube> skyboxTextures)
	{
		s_SceneData->skybox->Bind();
		s_SceneData->skyboxShader->Bind();
		s_SceneData->skyboxShader->SetInt("u_SkyBox", 0);
		s_SceneData->skyboxShader->SetMat4("u_View", s_SceneData->ViewMatrix);
		s_SceneData->skyboxShader->SetMat4("u_Projection", s_SceneData->ProjectionMatrix);


		//s_SceneData->skyboxMayerial->SetData<int>("u_SkyBox", 0);
		//s_SceneData->skyboxMayerial->SetData("u_View", s_SceneData->ViewMatrix);
		//s_SceneData->skyboxMayerial->SetData("u_Projection", s_SceneData->ProjectionMatrix);
		//s_SceneData->skyboxMayerial->SetData();
		skyboxTextures->Bind();

		RenderCommand::DrawIndexed(s_SceneData->skybox);
	}

	void Renderer::SubmitEnvironment(Ref<Environment> environment)
	{

		s_SceneData->environment = environment;
	}

	void Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
	{
		s_SceneData->activePass = renderPass;
		s_SceneData->activePass->GetSpecification().targetFrameBuffer->Bind();

		if (clear)
		{
			const glm::vec4& clearColor = s_SceneData->activePass->GetSpecification().targetFrameBuffer->GetSpecification().clearColor;
			RenderCommand::SetClearColor(clearColor);
			RenderCommand::Clear();
		}
	}

	void Renderer::EndRenderPass()
	{
		s_SceneData->activePass->GetSpecification().targetFrameBuffer->Unbind();
		s_SceneData->activePass = nullptr;
	}


	void Renderer::GeometryPass()
	{
		Renderer::BeginRenderPass(s_SceneData->geometryPass, true);

		int textureValue = -1;
		s_SceneData->geometryPass->GetSpecification().targetFrameBuffer->ClearAttachment(1, (void*)&textureValue);


		if (s_SceneData->environment)
		{

			RenderCommand::SetDepthMask(false);
			//Draw SkyBox
			s_SceneData->skyboxShader->Bind();
			s_SceneData->skyboxShader->SetInt("u_Skybox", 0);
			s_SceneData->skyboxShader->SetMat4("u_View", s_SceneData->ViewMatrix);
			s_SceneData->skyboxShader->SetMat4("u_Projection", s_SceneData->ProjectionMatrix);
			s_SceneData->environment->GetEnvironmentMap()->Bind();

			RenderCommand::DrawIndexed(s_SceneData->skybox);
			RenderCommand::SetDepthMask(true);

			//Draw mesh

			for (auto& dc : s_SceneData->drawList) {

				s_SceneData->textureSlotIndex = 0;

				dc.material->GetShader()->Bind();
				dc.material->Submit();
				s_SceneData->textureSlotIndex += dc.material->GetSampleUniformAmount();

				s_SceneData->environment->GetIrradianceMap()->Bind(s_SceneData->textureSlotIndex);
				dc.material->GetShader()->SetInt("u_EnvIrradiance", s_SceneData->textureSlotIndex++);

				dc.material->GetShader()->SetInt("u_BRDFLUT", s_SceneData->textureSlotIndex);
				Environment::GetBRDFLUT()->Bind(s_SceneData->textureSlotIndex++);

				s_SceneData->environment->GetEnvironmentMap()->Bind(s_SceneData->textureSlotIndex);
				dc.material->GetShader()->SetInt("u_EnvRadiance", s_SceneData->textureSlotIndex++);

				dc.material->GetShader()->SetInt("u_EntityID", dc.entityHandle);

				if (dc.subMesh->isRigged)
				{
					dc.material->GetShader()->SetInt("u_IsAnimation", true);
					for (uint32_t i = 0; i < dc.mesh->GetBoneAmount(); i++) {
						std::string BoneTransformUniformString = "u_BoneTransforms[" + std::to_string(i) + "]";
						dc.material->GetShader()->SetMat4(BoneTransformUniformString, dc.mesh->GetBoneTransform(i));
					}
				}
				else {
					dc.material->GetShader()->SetInt("u_IsAnimation", false);
				}


				dc.material->Submit();
				dc.material->GetShader()->SetMat4("u_Transform", dc.transform);
				RenderCommand::DrawIndexed(dc.subMesh->vertexArray);


			}
		}
		else {
			for (auto& dc : s_SceneData->drawList) {

				std::vector<Ref<SubMesh>> subMeshes = dc.mesh->GetSubMeshes();
				std::vector<Ref<Material>> materials = dc.mesh->GetMaterials();


				for (auto subMesh : subMeshes) {
					s_SceneData->textureSlotIndex = 0;

					Ref<Material> material = materials[subMesh->materialIndex];

					material->GetShader()->Bind();
					material->Submit();
					s_SceneData->textureSlotIndex += dc.material->GetSampleUniformAmount();
					dc.material->GetShader()->SetInt("u_EntityID", dc.entityHandle);
					if (dc.mesh->IsAnimated())
					{
						material->GetShader()->SetInt("u_IsAnimation", true);
						for (uint32_t i = 0; i < dc.mesh->GetBoneAmount(); i++) {
							std::string BoneTransformUniformString = "u_BoneTransforms[" + std::to_string(i) + "]";
							material->GetShader()->SetMat4(BoneTransformUniformString, dc.mesh->GetBoneTransform(i));
						}
					}
					else {
						material->GetShader()->SetInt("u_IsAnimation", false);
					}

					material->Submit();
					material->GetShader()->SetMat4("u_Transform", dc.transform);
					RenderCommand::DrawIndexed(subMesh->vertexArray);
				}
			}
		}
		Renderer::EndRenderPass();
	}


	void Renderer::CompositePass()
	{
		Renderer::BeginRenderPass(s_SceneData->compositePass);

		s_SceneData->geometryPass->GetSpecification().targetFrameBuffer->BindTexture(0);
		s_SceneData->compositeShader->Bind();
		s_SceneData->compositeShader->SetInt("u_Texture", 0);
		s_SceneData->fullScreenQuad->Bind();

		RenderCommand::DrawIndexed(s_SceneData->fullScreenQuad);

		Renderer::EndRenderPass();

	}

	void Renderer::FlushDrawList()
	{

		GeometryPass();
		CompositePass();

	}

	Ref<Framebuffer> Renderer::GetGeometryPassFramebuffer()
	{
		return s_SceneData->geometryPass->GetSpecification().targetFrameBuffer;
	}

	Ref<Framebuffer> Renderer::GetCompositePassFramebuffer()
	{
		return s_SceneData->compositePass->GetSpecification().targetFrameBuffer;
	}

	void Renderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		s_SceneData->compositePass->GetSpecification().targetFrameBuffer->Resize(glm::vec2{ width, height });
		s_SceneData->compositePass->GetSpecification().targetFrameBuffer->Resize(glm::vec2{ width, height });
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewPort(0, 0, width, height);
	}
	Ref<Material> Renderer::GetDefaultPBRMaterial()
	{
		return s_SceneData->defaultPBRMaterial;
	}
	Ref<Material> Renderer::GetDefaultPhongMaterial()
	{
		return s_SceneData->defaultPhongMaterial;
	}
	Ref<Shader> Renderer::GetDefaultPBRShader()
	{
		return  s_SceneData->defaultShader;
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