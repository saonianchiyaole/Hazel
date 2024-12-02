#include "hzpch.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Hazel/Renderer/RenderCommand.h"

#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	Renderer2D::RendererState Renderer2D::m_RendererState;


	struct QuadVertex {
		glm::vec3 position;
		glm::vec2 texCrood;
		glm::vec4 color;
		int texIndex;
		int entityID;
	};


	struct Renderer2DStorage {
		Ref<VertexArray> quadVertexArray;
		Ref<VertexBuffer> quadVertexBuffer;
		Ref<Shader> textureshader;
		Ref<Shader> flatshader;

		const uint32_t maxQuad = 10000;
		const uint32_t maxQuadVetices = maxQuad * 4;
		const uint32_t maxQuadIndices = maxQuad * 6;

		uint32_t QuadIndexCount = 0;

		QuadVertex* quadVertexBufferPtr = nullptr;
		QuadVertex* quadVertexBuffeBase = nullptr;

		static const uint32_t maxTextureSlot = 20;
		std::array<Ref<Texture2D>, maxTextureSlot> textureSlots;
		uint32_t textureSlotIndex = 1;

		Ref<Texture2D> whiteTexture;

		std::array<glm::vec4, 4> QuadVertices;
		std::array<glm::vec2, 4> QuadTextureCrood;

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

		m_Data->QuadVertices = {
			glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f},
			glm::vec4{0.5f,  -0.5f, 0.0f, 1.0f},
			glm::vec4{-0.5f,   0.5f, 0.0f, 1.0f},
			glm::vec4{0.5f,  0.5f, 0.0f, 1.0f}
		};
		m_Data->QuadTextureCrood = {
			glm::vec2{0, 0},
			glm::vec2{1, 0},
			glm::vec2{0, 1},
			glm::vec2{1, 1}
		};

		m_Data->QuadIndexCount = 0;
		m_Data->quadVertexBufferPtr = nullptr;

		m_Data->textureSlotIndex = 1;

		//Vertex
		m_Data->quadVertexArray = Hazel::VertexArray::Create();


		m_Data->quadVertexBuffer = Hazel::VertexBuffer::Create(m_Data->maxQuadVetices * sizeof(QuadVertex));

		m_Data->quadVertexBuffeBase = new QuadVertex[m_Data->maxQuadVetices];

		Hazel::BufferLayout layout = std::vector<Hazel::BufferElement>{
			{ Hazel::ShaderDataType::Float3, "a_Position"},
			{ Hazel::ShaderDataType::Float2, "a_TexCrood"},
			{ Hazel::ShaderDataType::Float4, "a_Color"},
			{ Hazel::ShaderDataType::Int, "a_TexIndex"},
			{ Hazel::ShaderDataType::Int, "a_EntityID"},
		};
		m_Data->quadVertexBuffer->SetLayout(layout);


		m_Data->quadVertexArray->AddVertexBuffer(m_Data->quadVertexBuffer);

		uint32_t* quadIndices = new uint32_t[m_Data->maxQuadIndices];

		uint32_t offset = 0;
		for (int i = 0; i < m_Data->maxQuadIndices; i += 6, offset += 4)
		{
			quadIndices[i + 0] = 0 + offset;
			quadIndices[i + 1] = 1 + offset;
			quadIndices[i + 2] = 2 + offset;

			quadIndices[i + 3] = 2 + offset;
			quadIndices[i + 4] = 3 + offset;
			quadIndices[i + 5] = 1 + offset;
		}

		unsigned int indices[6] = { 0, 1, 3, 3, 2, 1 };
		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer = Hazel::IndexBuffer::Create(quadIndices, m_Data->maxQuadIndices);
		m_Data->quadVertexArray->SetIndexBuffer(indexBuffer);
		delete[] quadIndices;

		//Default texture

		//Shader
		Hazel::ShaderLibrary::Load("assets/Shaders/Texture.glsl");
		m_Data->textureshader = Hazel::ShaderLibrary::Get("Texture");


		//Default white texture
		m_Data->whiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		m_Data->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		m_Data->textureSlots[0] = m_Data->whiteTexture;
		//Allocate slot
		m_Data->textureshader->Bind();
		int* indexArray = new int[m_Data->maxTextureSlot];
		for (int i = 0; i < m_Data->maxTextureSlot; i++) {
			indexArray[i] = i;
		}
		m_Data->textureshader->SetIntArray("u_Textures", indexArray, m_Data->maxTextureSlot);
	}
	void Renderer2D::Shutdown()
	{
		delete m_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		//Initialize variable


		m_Data->textureshader->Bind();
		m_Data->textureshader->SetMat4("u_View", camera.GetViewMatrix());
		m_Data->textureshader->SetMat4("u_Projection", camera.GetProjectionMatrix());
		m_Data->textureshader->SetMat4("u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)));

		m_Data->quadVertexBufferPtr = m_Data->quadVertexBuffeBase;

	}
	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		m_Data->textureshader->Bind();
		m_Data->textureshader->SetMat4("u_View", camera.GetViewMatrix());
		m_Data->textureshader->SetMat4("u_Projection", camera.GetProjectionMatrix());
		m_Data->textureshader->SetMat4("u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)));

		m_Data->quadVertexBufferPtr = m_Data->quadVertexBuffeBase;
	}
	void Renderer2D::EndScene()
	{


		Flush();

		m_Data->QuadIndexCount = 0;
		m_Data->quadVertexBufferPtr = nullptr;

		m_Data->textureSlotIndex = 1;
	}

	void Renderer2D::Flush()
	{

		uint32_t dataSize = m_Data->quadVertexBufferPtr - m_Data->quadVertexBuffeBase;
		m_Data->quadVertexBuffer->SetData(m_Data->quadVertexBuffeBase, dataSize * sizeof(QuadVertex));

		for (uint32_t i = m_Data->textureSlotIndex; i < m_Data->maxTextureSlot; i++) {
			m_Data->textureSlots[i] = m_Data->whiteTexture;
			m_Data->textureSlots[i]->Bind();
		}


		for (uint32_t i = 0; i < m_Data->textureSlotIndex; i++) {
			m_Data->textureSlots[i]->Bind(i);
		}


		m_Data->quadVertexArray->Bind();
		RenderCommand::DrawIndexed(m_Data->quadVertexArray, m_Data->QuadIndexCount);


		m_RendererState.drawCall += 1;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		for (int i = 0; i < 4; i++) {
			m_Data->quadVertexBufferPtr->position = transform * m_Data->QuadVertices[i];
			m_Data->quadVertexBufferPtr->color = color;
			m_Data->quadVertexBufferPtr->texCrood = m_Data->QuadTextureCrood[i];
			m_Data->quadVertexBufferPtr->texIndex = 0;
			m_Data->quadVertexBufferPtr->entityID = entityID;
			m_Data->quadVertexBufferPtr++;
		}

		m_Data->QuadIndexCount += 6;

		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, Ref<Texture2D> texture, int entityID)
	{
		int textureSlotIndex = 0;
		for (GLint i = 1; i < m_Data->textureSlotIndex; i++) {
			if (*texture.get() == *m_Data->textureSlots[i].get())
			{
				textureSlotIndex = i;
				break;
			}
		}

		if (textureSlotIndex == 0) {
			textureSlotIndex = (int)m_Data->textureSlotIndex;
			m_Data->textureSlots[m_Data->textureSlotIndex++] = texture;
		}

		glm::vec4 color = glm::vec4(1.0f);

		for (int i = 0; i < 4; i++) {
			m_Data->quadVertexBufferPtr->position = transform * m_Data->QuadVertices[i];
			m_Data->quadVertexBufferPtr->color = color;
			m_Data->quadVertexBufferPtr->texCrood = m_Data->QuadTextureCrood[i];
			m_Data->quadVertexBufferPtr->texIndex = (GLint)textureSlotIndex;
			m_Data->quadVertexBufferPtr->entityID = entityID;
			m_Data->quadVertexBufferPtr++;
		}

		m_Data->QuadIndexCount += 6;
		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, int entityID)
	{
		int textureSlotIndex = 0;
		for (GLint i = 1; i < m_Data->textureSlotIndex; i++) {
			if (*texture.get() == *m_Data->textureSlots[i].get())
			{
				textureSlotIndex = i;
				break;
			}
		}

		if (textureSlotIndex == 0) {
			textureSlotIndex = (int)m_Data->textureSlotIndex;
			m_Data->textureSlots[m_Data->textureSlotIndex++] = texture;
		}

		for (int i = 0; i < 4; i++) {
			m_Data->quadVertexBufferPtr->position = transform * m_Data->QuadVertices[i];
			m_Data->quadVertexBufferPtr->color = color;
			m_Data->quadVertexBufferPtr->texCrood = m_Data->QuadTextureCrood[i];
			m_Data->quadVertexBufferPtr->texIndex = (GLint)textureSlotIndex;
			m_Data->quadVertexBufferPtr->entityID = entityID;
			m_Data->quadVertexBufferPtr++;
		}

		m_Data->QuadIndexCount += 6;
		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}


	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad(glm::vec3{ position, 0.0f }, size, color);
	}



	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size, 0));

		for (int i = 0; i < 4; i++) {
			m_Data->quadVertexBufferPtr->position = transform * m_Data->QuadVertices[i];
			m_Data->quadVertexBufferPtr->color = color;
			m_Data->quadVertexBufferPtr->texCrood = m_Data->QuadTextureCrood[i];
			m_Data->quadVertexBufferPtr->texIndex = 0;
			m_Data->quadVertexBufferPtr++;
		}

		m_Data->QuadIndexCount += 6;

		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float angle, const glm::vec4& color)
	{
		DrawRotatedQuad(glm::vec3(position, 0.0f), size, angle, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float angle, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size, 0));

		for (int i = 0; i < 4; i++) {
			m_Data->quadVertexBufferPtr->position = transform * m_Data->QuadVertices[i];
			m_Data->quadVertexBufferPtr->color = color;
			m_Data->quadVertexBufferPtr->texCrood = m_Data->QuadTextureCrood[i];
			m_Data->quadVertexBufferPtr->texIndex = 0;
			m_Data->quadVertexBufferPtr++;
		}

		m_Data->QuadIndexCount += 6;

		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, Ref<Texture2D>& texture)
	{
		DrawQuad(glm::vec3{ position, 0.0f }, size, texture);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<Texture2D>& texture)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size, 0));

		int textureSlotIndex = 0;
		for (GLint i = 1; i < m_Data->textureSlotIndex; i++) {
			if (*texture.get() == *m_Data->textureSlots[i].get())
			{
				textureSlotIndex = i;
				break;
			}
		}

		if (textureSlotIndex == 0) {
			textureSlotIndex = (int)m_Data->textureSlotIndex;
			m_Data->textureSlots[m_Data->textureSlotIndex++] = texture;
		}


		glm::vec4 color = glm::vec4(1.0f);



		for (int i = 0; i < 4; i++) {
			m_Data->quadVertexBufferPtr->position = transform * m_Data->QuadVertices[i];
			m_Data->quadVertexBufferPtr->color = color;
			m_Data->quadVertexBufferPtr->texCrood = m_Data->QuadTextureCrood[i];
			m_Data->quadVertexBufferPtr->texIndex = (GLint)textureSlotIndex;
			m_Data->quadVertexBufferPtr++;
		}

		m_Data->QuadIndexCount += 6;
		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}
	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float angle, Ref<Texture2D>& texture)
	{
		DrawRotatedQuad(glm::vec3(position, 0.0f), size, angle, texture);
	}
	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float angle, Ref<Texture2D>& texture)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size, 0));


		glm::vec4 color = glm::vec4(1.0f);

		int textureSlotIndex = 0;
		for (GLint i = 1; i < m_Data->textureSlotIndex; i++) {
			if (*texture.get() == *m_Data->textureSlots[i].get())
			{
				textureSlotIndex = i;
				break;
			}
		}

		if (textureSlotIndex == 0) {
			textureSlotIndex = (int)m_Data->textureSlotIndex;
			m_Data->textureSlots[m_Data->textureSlotIndex++] = texture;
		}

		for (int i = 0; i < 4; i++) {
			m_Data->quadVertexBufferPtr->position = transform * m_Data->QuadVertices[i];
			m_Data->quadVertexBufferPtr->color = color;
			m_Data->quadVertexBufferPtr->texCrood = m_Data->QuadTextureCrood[i];
			m_Data->quadVertexBufferPtr->texIndex = (GLint)textureSlotIndex;
			m_Data->quadVertexBufferPtr++;
		}

		m_Data->QuadIndexCount += 6;
		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}
	void Renderer2D::DrawSprite(const glm::mat4& transform, const SpriteComponent& sprite, const int entityID)
	{
		if (sprite.texture)
			DrawQuad(transform, sprite.color, sprite.texture, entityID);
		else
			DrawQuad(transform, sprite.color, entityID);
	}
	const Renderer2D::RendererState* Renderer2D::GetState()
	{
		return &m_RendererState;
	}


	void Renderer2D::ResetState() {
		m_RendererState.quadAmount = 0;
		m_RendererState.vertexAmount = 0;
		m_RendererState.indexAmount = 0;
		m_RendererState.drawCall = 0;
	}
}