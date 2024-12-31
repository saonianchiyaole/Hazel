#include "hzpch.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Scene/Component.h"
#include "Hazel/Renderer/VertexArray.h"

namespace Hazel {

	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

	void Renderer::Init() {
		RenderCommand::Init();
		Renderer2D::Init();

		s_SceneData->cameraUniformBuffer = UniformBuffer::Create(sizeof(glm::mat4), 0);
		ShaderLibrary::Load("assets/Shaders/Shader.glsl");
		s_SceneData->defaultShader = ShaderLibrary::Get("Shader");
	}

	void Renderer::BeginScene(const Camera& camera)
	{
		s_SceneData->ViewMatrix = camera.GetViewMatrix();
		s_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();

		s_SceneData->cameraUniformBuffer->SetData(&camera.GetViewProjectionMatrix(), sizeof(glm::mat4), 0);
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		s_SceneData->ViewMatrix = camera.GetViewMatrix();
		s_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();

		s_SceneData->cameraUniformBuffer->SetData(&camera.GetViewProjectionMatrix(), sizeof(glm::mat4), 0);
	}

	void Renderer::EndScene()
	{
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
		shader->SetInt("u_Albedo", 0);
		mesh->GetTexture(TextureType::Albedo)->Bind(0);
		mesh->m_VertexArray->Bind();


		RenderCommand::DrawIndexed(mesh->m_VertexArray);

		
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewPort(0, 0, width, height);
	}
}