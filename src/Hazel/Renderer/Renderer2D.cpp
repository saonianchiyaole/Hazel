#include "hzpch.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Hazel/Renderer/RenderCommand.h"

#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	struct Renderer2DStorage {
		Ref<VertexArray> quadVertexArray;
		Ref<Shader> Textureshader;
		Ref<Shader> Flatshader;
	};

	static Renderer2DStorage* m_Data;

	float vertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  //bottom left
			0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,  //bottom right
			0.5f,   0.5f, 0.0f, 1.0f, 1.0f,  //top right
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f   //top left
	};
	void Renderer2D::Init()
	{

		m_Data = new Renderer2DStorage;

		//Vertex
		m_Data->quadVertexArray = Hazel::VertexArray::Create();

		std::shared_ptr<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));


		Hazel::BufferLayout layout = std::vector<Hazel::BufferElement>{
			{ Hazel::ShaderDataType::Float3, "a_Position"},
			{ Hazel::ShaderDataType::Float2, "v_TexCrood"}
		};
		vertexBuffer->SetLayout(layout);


		m_Data->quadVertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[6] = { 0, 1, 3, 3, 2, 1 };
		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(unsigned int)));
		m_Data->quadVertexArray->SetIndexBuffer(indexBuffer);

		//Shader
		Hazel::ShaderLibrary::Load("assets/Shaders/Texture.glsl");
		m_Data->Textureshader = Hazel::ShaderLibrary::Get("Texture");

		Hazel::ShaderLibrary::Load("assets/Shaders/FlatShader.glsl");
		m_Data->Flatshader = Hazel::ShaderLibrary::Get("FlatShader");
	}
	void Renderer2D::Shutdown()
	{
		delete m_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) 
	{
		m_Data->Textureshader->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_Data->Textureshader)->UploadUniformMat4("u_View", camera.GetViewMatrix());
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_Data->Textureshader)->UploadUniformMat4("u_Projection", camera.GetProjectionMatrix());
	
		m_Data->Flatshader->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_Data->Flatshader)->UploadUniformMat4("u_View", camera.GetViewMatrix());
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_Data->Flatshader)->UploadUniformMat4("u_Projection", camera.GetProjectionMatrix());

	}
	void Renderer2D::EndScene()
	{

	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad(glm::vec3{position, 0.0f}, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, Ref<Texture2D>& texture)
	{
		DrawQuad(glm::vec3{ position, 0.0f }, size, texture);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		m_Data->Flatshader->Bind();
		m_Data->Flatshader->SetFloat4("u_Color", color);
		m_Data->Flatshader->SetMat4("u_Transform", glm::translate(glm::mat4(1.0f), position));
		
		
		m_Data->quadVertexArray->Bind();
		RenderCommand::DrawIndexed(m_Data->quadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<Texture2D>& texture)
	{
		m_Data->Textureshader->Bind();
		m_Data->Textureshader->SetInt("u_Texture", 0);
		m_Data->Textureshader->SetMat4("u_Transform", glm::translate(glm::mat4(1.0f), position));
		texture->Bind();

		m_Data->quadVertexArray->Bind();
		RenderCommand::DrawIndexed(m_Data->quadVertexArray);
	}
}