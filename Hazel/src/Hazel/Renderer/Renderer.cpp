#include "hzpch.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Scene/Component.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Environment.h"


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



	void Renderer::Init() {
		RenderCommand::Init();
		Renderer2D::Init();

		s_SceneData->cameraUniformBuffer = UniformBuffer::Create(sizeof(CameraUniformBuffer), 0);
		s_SceneData->lightUniformBuffer = UniformBuffer::Create(sizeof(LightUniformBuffer), 1);

		ShaderLibrary::Load("assets/Shaders/Standard.glsl");
		s_SceneData->defaultShader = ShaderLibrary::Get("Standard");

		ShaderLibrary::Load("assets/Shaders/Skybox.glsl");
		s_SceneData->skyboxShader = ShaderLibrary::Get("Skybox");
		


		s_SceneData->skybox = VertexArray::Create();
		BufferLayout skyboxBufferLayout = std::vector<Hazel::BufferElement>{
			{Hazel::ShaderDataType::Float3, "a_Position" }
		};
		Ref<VertexBuffer> skyboxVertexBuffer = VertexBuffer::Create(skyboxVertices, sizeof(skyboxVertices));
		skyboxVertexBuffer->SetLayout(skyboxBufferLayout);
		s_SceneData->skybox->AddVertexBuffer(skyboxVertexBuffer);

		Ref<IndexBuffer> skyboxIndexBuffer = IndexBuffer::Create(skyboxIndices, sizeof(skyboxIndices) / sizeof(uint32_t));
		s_SceneData->skybox->SetIndexBuffer(skyboxIndexBuffer);

		//Set Dufault Light Uniform
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

		s_SceneData->skyboxShader->SetMat4("u_View", camera.GetViewMatrix());
		s_SceneData->skyboxShader->SetMat4("u_Projection", camera.GetProjectionMatrix());
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

		s_SceneData->skyboxShader->SetMat4("u_View", camera.GetViewMatrix());
		s_SceneData->skyboxShader->SetMat4("u_Projection", camera.GetProjectionMatrix());
	}

	void Renderer::EndScene()
	{
		s_SceneData->environment = nullptr;
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
		shader->Bind();
		shader->SetMat4("u_Transform", transformComponent.transform);
		mesh->m_VertexArray->Bind();


		RenderCommand::DrawIndexed(mesh->m_VertexArray);
	}

	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const TransformComponent& transformComponent, Ref<Material> material)
	{

		if (!material)
		{
			SubmitMesh(mesh, transformComponent, s_SceneData->defaultShader);
			return;
		}

		

		material->GetShader()->Bind();
		material->Submit();
		material->GetShader()->SetMat4("u_Transform", transformComponent.transform);
		
		if(s_SceneData->environment)
		{
			s_SceneData->environment->GetIrradianceMap()->Bind(10);
			material->GetShader()->SetInt("u_EnvIrradiance", 10);
		}
		mesh->m_VertexArray->Bind();


		RenderCommand::DrawIndexed(mesh->m_VertexArray);

	}



	void Renderer::SubmitLight(const LightComponent& lightComponent, const TransformComponent& transformComponent)
	{
		LightUniformBuffer lightUniformBufferData;
		lightUniformBufferData.position = { transformComponent.translate, 0.0f};

		glm::quat quaternion = glm::quat(transformComponent.rotation);
		glm::mat3 rotationMatrix = glm::toMat3(quaternion);

		lightUniformBufferData.direction = { glm::normalize(rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f)), 0.0f };
		lightUniformBufferData.color = {lightComponent.color, 0.0f};

		s_SceneData->lightUniformBuffer->SetData(&lightUniformBufferData, sizeof(LightUniformBuffer), 0); 
	}

	void Renderer::SubmitSkybox(Ref<TextureCube> skyboxTextures)
	{
		RenderCommand::SetDepthMask(false);
		s_SceneData->skybox->Bind();
		s_SceneData->skyboxShader->Bind();
		s_SceneData->skyboxShader->SetInt("u_SkyBox", 0);
		s_SceneData->skyboxShader->SetMat4("u_View", s_SceneData->ViewMatrix);
		s_SceneData->skyboxShader->SetMat4("u_Projection", s_SceneData->ProjectionMatrix);
		skyboxTextures->Bind();
		
		RenderCommand::DrawIndexed(s_SceneData->skybox);

		RenderCommand::SetDepthMask(true);
	}

	void Renderer::SubmitEnvironment(Ref<Environment> environment)
	{
		RenderCommand::SetDepthMask(false);

		s_SceneData->skybox->Bind();
		s_SceneData->skyboxShader->Bind();
		s_SceneData->skyboxShader->SetInt("u_SkyBox", 0);
		s_SceneData->skyboxShader->SetMat4("u_View", s_SceneData->ViewMatrix);
		s_SceneData->skyboxShader->SetMat4("u_Projection", s_SceneData->ProjectionMatrix);
		environment->GetEnvironmentMap()->Bind();
		s_SceneData->environment = environment;

		RenderCommand::DrawIndexed(s_SceneData->skybox);

		RenderCommand::SetDepthMask(true);

	}


	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewPort(0, 0, width, height);
	}
}