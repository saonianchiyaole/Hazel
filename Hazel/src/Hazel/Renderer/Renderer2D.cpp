#include "hzpch.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Hazel/Renderer/RenderCommand.h"

#include "Hazel/Core/Application.h"

#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Pipeline.h"
#include "Hazel/Renderer/Swapchain.h"
#include "Hazel/Renderer/RenderPass.h"

namespace Hazel {

	Renderer2D::RendererState Renderer2D::m_RendererState;


	struct QuadVertex {
		glm::vec3 position;
		glm::vec2 texCrood;
		glm::vec4 color;
		int texIndex;		
	};

	struct CircleVertex {
		glm::vec3 worldPosition;
		glm::vec3 localPosition;
		glm::vec4 color;
		float thickness;
		float fade;		
	};

	struct LineVertex {
		glm::vec3 position;
		glm::vec4 color;		
	};

	struct Renderer2DStorage {
		Ref<VertexArray> quadVertexArray;
		Ref<VertexBuffer> quadVertexBuffer;
		Ref<Pipeline> texturePipeline;
		Ref<RenderPass> textureRenderPass;

		Ref<VertexArray> circleVertexArray;
		Ref<VertexBuffer> circleVertexBuffer;
		Ref<Pipeline> circlePipeline;
		Ref<RenderPass> circleRenderPass;

		Ref<VertexArray> lineVertexArray;
		Ref<VertexBuffer> lineVertexBuffer;
		Ref<Pipeline> linePipeline;
		Ref<RenderPass> lineRenderPass;

		Ref<Shader> textureShader;
		Ref<Shader> circleShader;
		Ref<Shader> lineShader;		
		
		const uint32_t maxQuad = 10000;
		const uint32_t maxQuadVetices = maxQuad * 4;
		const uint32_t maxQuadIndices = maxQuad * 6;

		uint32_t QuadIndexCount = 0;
		uint32_t circleIndexCount = 0;
		uint32_t lineIndexCount = 0;

		QuadVertex* quadVertexBufferPtr = nullptr;
		QuadVertex* quadVertexBuffeBase = nullptr;

		CircleVertex* circleVertexBuffePtr = nullptr;
		CircleVertex* circleVertexBuffeBase = nullptr;

		LineVertex* lineVertexBuffePtr = nullptr;
		LineVertex* lineVertexBuffeBase = nullptr;

		float lineWidth = 1;

		static const uint32_t maxTextureSlot = 20;
		std::array<Ref<Texture2D>, maxTextureSlot> textureSlots;
		uint32_t textureSlotIndex = 1;

		Ref<Texture2D> whiteTexture;

		std::array<glm::vec4, 4> QuadVertices;
		std::array<glm::vec2, 4> QuadTextureCrood;

		struct CameraUniformBuffer {
			glm::mat4 viewProjectionMatrix;
			//glm::vec4 position;
		};
		CameraUniformBuffer cameraBuffer;
		Ref<UniformBuffer> cameraUniformBuffer;


		Ref<Framebuffer> frameBuffer;
						
		
	};

	static Renderer2DStorage* s_Data;

	float vertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  //bottom left
			0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,  //bottom right
			0.5f,   0.5f, 0.0f, 1.0f, 1.0f,  //top right
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f   //top left
	};

	void Renderer2D::Init()
	{

		s_Data = new Renderer2DStorage;

		s_Data->QuadVertices = {
			glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f},
			glm::vec4{0.5f,  -0.5f, 0.0f, 1.0f},
			glm::vec4{-0.5f,  0.5f, 0.0f, 1.0f},
			glm::vec4{0.5f,  0.5f, 0.0f, 1.0f}
		};
		s_Data->QuadTextureCrood = {
			glm::vec2{0, 0},
			glm::vec2{1, 0},
			glm::vec2{0, 1},
			glm::vec2{1, 1}
		};

		s_Data->QuadIndexCount = 0;
		s_Data->quadVertexBufferPtr = nullptr;
		s_Data->circleIndexCount = 0;
		s_Data->circleVertexBuffePtr = nullptr;

		s_Data->textureSlotIndex = 1;


		uint32_t* quadIndices = new uint32_t[s_Data->maxQuadIndices];

		uint32_t offset = 0;
		for (int i = 0; i < s_Data->maxQuadIndices; i += 6, offset += 4)
		{
			quadIndices[i + 0] = 0 + offset;
			quadIndices[i + 1] = 1 + offset;
			quadIndices[i + 2] = 2 + offset;

			quadIndices[i + 3] = 2 + offset;
			quadIndices[i + 4] = 3 + offset;
			quadIndices[i + 5] = 1 + offset;
		}


		Ref<IndexBuffer> quadIndexBuffer;
		quadIndexBuffer = IndexBuffer::Create(quadIndices, s_Data->maxQuadIndices);

		//Shader
		{
			s_Data->textureShader = ShaderLibrary::Load("assets/Shaders/2DQuadShader.glsl");
			s_Data->circleShader = ShaderLibrary::Load("assets/Shaders/2DCircleShader.glsl");			
			s_Data->lineShader = ShaderLibrary::Load("assets/Shaders/2DLineShader.glsl");
		}


		FramebufferSpecification fbSpec;
		fbSpec.width = Application::GetInstance().GetWindow().GetWidth();
		fbSpec.height = Application::GetInstance().GetWindow().GetHeight();
		fbSpec.attachments = { TextureFormat::RGBA, TextureFormat::Depth };
		s_Data->frameBuffer = Framebuffer::Create(fbSpec);


		//QuadVertex Start
		{
			s_Data->quadVertexArray = VertexArray::Create();
			s_Data->quadVertexBuffer = VertexBuffer::Create(s_Data->maxQuadVetices * sizeof(QuadVertex));
			s_Data->quadVertexBuffeBase = new QuadVertex[s_Data->maxQuadVetices];

			BufferLayout layout = std::vector<BufferElement>{
				{ ShaderDataType::Float3, "a_Position"},
				{ ShaderDataType::Float2, "a_TexCrood"},
				{ ShaderDataType::Float4, "a_Color"},
				{ ShaderDataType::Int, "a_TexIndex"},
			};
			s_Data->quadVertexBuffer->SetLayout(layout);
			s_Data->quadVertexArray->AddVertexBuffer(s_Data->quadVertexBuffer);		
			s_Data->quadVertexArray->SetIndexBuffer(quadIndexBuffer);

			PipelineSpecification pipelineSpec;
			pipelineSpec.bufferLayout = layout;
			pipelineSpec.shader = s_Data->textureShader;
			pipelineSpec.targetFramebuffer = s_Data->frameBuffer;
			s_Data->texturePipeline = Pipeline::Create(pipelineSpec);
			
			RenderPassSpecification renderPassSpec;
			renderPassSpec.pipeline = s_Data->texturePipeline;			
			s_Data->textureRenderPass = RenderPass::Create(renderPassSpec);

		}
		//QuadVertex End

		//CircleVertex Start
		{
			s_Data->circleVertexArray = VertexArray::Create();
			s_Data->circleVertexBuffer = VertexBuffer::Create(s_Data->maxQuadVetices * sizeof(CircleVertex));
			s_Data->circleVertexBuffeBase = new CircleVertex[s_Data->maxQuadVetices];

			BufferLayout circleLayout = std::vector<BufferElement>{
				{ ShaderDataType::Float3, "a_WorldPosition"},
				{ ShaderDataType::Float3, "a_LocalPosition"},
				{ ShaderDataType::Float4, "a_Color"},
				{ ShaderDataType::Float, "a_Thickness"},
				{ ShaderDataType::Float, "a_Fade"},				
			};
			s_Data->circleVertexBuffer->SetLayout(circleLayout);
			s_Data->circleVertexArray->AddVertexBuffer(s_Data->circleVertexBuffer);
			s_Data->circleVertexArray->SetIndexBuffer(quadIndexBuffer);

			PipelineSpecification pipelineSpec;
			pipelineSpec.bufferLayout = circleLayout;
			pipelineSpec.shader = s_Data->circleShader;
			pipelineSpec.targetFramebuffer = s_Data->frameBuffer;
			s_Data->circlePipeline = Pipeline::Create(pipelineSpec);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.pipeline = s_Data->texturePipeline;
			s_Data->circleRenderPass = RenderPass::Create(renderPassSpec);


			delete[] quadIndices;
		}
		//CircleVertex End


		//LineVertex Start
		{
			s_Data->lineVertexArray = VertexArray::Create();
			s_Data->lineVertexBuffer = VertexBuffer::Create(s_Data->maxQuadVetices * sizeof(LineVertex));
			s_Data->lineVertexBuffeBase = new LineVertex[s_Data->maxQuadVetices];
			BufferLayout lineLayout = std::vector<BufferElement>{
				{ ShaderDataType::Float3, "a_Position"},
				{ ShaderDataType::Float4, "a_Color"},				
			};
			s_Data->lineVertexBuffer->SetLayout(lineLayout);
			s_Data->lineVertexArray->AddVertexBuffer(s_Data->lineVertexBuffer);


			PipelineSpecification pipelineSpec;
			pipelineSpec.bufferLayout = lineLayout;
			pipelineSpec.shader = s_Data->lineShader;
			pipelineSpec.targetFramebuffer = s_Data->frameBuffer;
			s_Data->linePipeline = Pipeline::Create(pipelineSpec);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.pipeline = s_Data->texturePipeline;
			s_Data->lineRenderPass = RenderPass::Create(renderPassSpec);

		}
		//LineVertex End

						
		//Default white texture
		{
			s_Data->whiteTexture = Texture2D::Create(1, 1);
			uint32_t whiteTextureData = 0xffffffff;
			s_Data->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
			s_Data->textureSlots[0] = s_Data->whiteTexture;
			//Allocate slot
			s_Data->textureShader->Bind();
			int* indexArray = new int[s_Data->maxTextureSlot];
			for (int i = 0; i < s_Data->maxTextureSlot; i++) {
				indexArray[i] = i;
			}
			s_Data->textureShader->SetIntArray("u_Textures", indexArray, s_Data->maxTextureSlot);
		}

		
		s_Data->cameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DStorage::CameraUniformBuffer), 0);

	}
	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		//Initialize variable
		//Renderer::GetCompositePassFramebuffer()->Bind();

		Renderer2DStorage::CameraUniformBuffer cameraUniformBufferData;
		cameraUniformBufferData.viewProjectionMatrix = camera.GetViewProjectionMatrix();
		//cameraUniformBufferData.position = { camera.GetPosition(), 0.0f };
		s_Data->cameraUniformBuffer->SetData((const void*)&s_Data->cameraBuffer, sizeof(Renderer2DStorage::CameraUniformBuffer), 0);
		
		s_Data->quadVertexBufferPtr = s_Data->quadVertexBuffeBase;
		s_Data->circleVertexBuffePtr = s_Data->circleVertexBuffeBase;
		s_Data->lineVertexBuffePtr = s_Data->lineVertexBuffeBase;

	}
	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		// todo 
		//Renderer::GetCompositePassFramebuffer()->Bind();

		Renderer2DStorage::CameraUniformBuffer cameraUniformBufferData;
		cameraUniformBufferData.viewProjectionMatrix = camera.GetViewProjectionMatrix();
		//cameraUniformBufferData.position = { camera.GetPosition(), 0.0f };
		s_Data->cameraUniformBuffer->SetData((const void*)&s_Data->cameraBuffer, sizeof(Renderer2DStorage::CameraUniformBuffer), 0);

		s_Data->quadVertexBufferPtr = s_Data->quadVertexBuffeBase;
		s_Data->circleVertexBuffePtr = s_Data->circleVertexBuffeBase;
		s_Data->lineVertexBuffePtr = s_Data->lineVertexBuffeBase;
	}

	void Renderer2D::BeginScene(const Camera& camera)
	{
		//Renderer::GetCompositePassFramebuffer()->Bind();

		Renderer2DStorage::CameraUniformBuffer cameraUniformBufferData;
		cameraUniformBufferData.viewProjectionMatrix = camera.GetViewProjectionMatrix();
		//cameraUniformBufferData.position = { camera.GetPosition(), 0.0f };
		s_Data->cameraUniformBuffer->SetData((const void*)&cameraUniformBufferData, sizeof(Renderer2DStorage::CameraUniformBuffer), 0);

		s_Data->quadVertexBufferPtr = s_Data->quadVertexBuffeBase;
		s_Data->circleVertexBuffePtr = s_Data->circleVertexBuffeBase;
		s_Data->lineVertexBuffePtr = s_Data->lineVertexBuffeBase;
	}

	void Renderer2D::EndScene()
	{


		Flush();

		s_Data->QuadIndexCount = 0;
		s_Data->circleIndexCount = 0;
		s_Data->lineIndexCount = 0;
		s_Data->quadVertexBufferPtr = s_Data->quadVertexBuffeBase;
		s_Data->circleVertexBuffePtr = s_Data->circleVertexBuffeBase;
		s_Data->lineVertexBuffePtr = s_Data->lineVertexBuffeBase;

		s_Data->textureSlotIndex = 1;

		//Renderer::GetCompositePassFramebuffer()->Unbind();
	}

	void Renderer2D::Flush()
	{

		

		for (uint32_t i = s_Data->textureSlotIndex; i < s_Data->maxTextureSlot; i++) {
			s_Data->textureSlots[i] = s_Data->whiteTexture;
			s_Data->textureSlots[i]->Bind();
		}


		for (uint32_t i = 0; i < s_Data->textureSlotIndex; i++) {
			s_Data->textureSlots[i]->Bind(i);
		}

		if (s_Data->QuadIndexCount){
			uint32_t dataSize = s_Data->quadVertexBufferPtr - s_Data->quadVertexBuffeBase;
			s_Data->quadVertexBuffer->SetData(s_Data->quadVertexBuffeBase, dataSize * sizeof(QuadVertex));

			s_Data->textureShader->Bind();
			s_Data->quadVertexArray->Bind();
			RenderCommand::DrawIndexed(s_Data->quadVertexArray, s_Data->QuadIndexCount);
			m_RendererState.drawCall += 1;
		}

		if (s_Data->circleIndexCount) {
			uint32_t dataSize = s_Data->circleVertexBuffePtr - s_Data->circleVertexBuffeBase;
			s_Data->circleVertexBuffer->SetData(s_Data->circleVertexBuffeBase, dataSize * sizeof(CircleVertex));

			s_Data->circleShader->Bind();
			s_Data->circleVertexArray->Bind();
			RenderCommand::DrawIndexed(s_Data->circleVertexArray, s_Data->circleIndexCount);
			m_RendererState.drawCall += 1;
		}

		if (s_Data->lineIndexCount) {
			uint32_t dataSize = s_Data->lineVertexBuffePtr - s_Data->lineVertexBuffeBase;
			s_Data->lineVertexBuffer->SetData(s_Data->lineVertexBuffeBase, dataSize * sizeof(LineVertex));

			s_Data->lineShader->Bind();
			s_Data->lineVertexArray->Bind();
			RenderCommand::SetLineWidth(s_Data->lineWidth);
			RenderCommand::DrawLines(s_Data->lineVertexArray, s_Data->lineIndexCount);
			m_RendererState.drawCall += 1;
		}
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		for (int i = 0; i < 4; i++) {
			s_Data->quadVertexBufferPtr->position = transform * s_Data->QuadVertices[i];
			s_Data->quadVertexBufferPtr->color = color;
			s_Data->quadVertexBufferPtr->texCrood = s_Data->QuadTextureCrood[i];
			s_Data->quadVertexBufferPtr->texIndex = 0;			
			s_Data->quadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;

		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, Ref<Texture2D> texture, int entityID)
	{
		int textureSlotIndex = 0;
		for (GLint i = 1; i < s_Data->textureSlotIndex; i++) {
			if (*texture.get() == *s_Data->textureSlots[i].get())
			{
				textureSlotIndex = i;
				break;
			}
		}

		if (textureSlotIndex == 0) {
			textureSlotIndex = (int)s_Data->textureSlotIndex;
			s_Data->textureSlots[s_Data->textureSlotIndex++] = texture;
		}

		glm::vec4 color = glm::vec4(1.0f);

		for (int i = 0; i < 4; i++) {
			s_Data->quadVertexBufferPtr->position = transform * s_Data->QuadVertices[i];
			s_Data->quadVertexBufferPtr->color = color;
			s_Data->quadVertexBufferPtr->texCrood = s_Data->QuadTextureCrood[i];
			s_Data->quadVertexBufferPtr->texIndex = (GLint)textureSlotIndex;			
			s_Data->quadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;
		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, int entityID)
	{
		int textureSlotIndex = 0;
		for (GLint i = 1; i < s_Data->textureSlotIndex; i++) {
			if (*texture.get() == *s_Data->textureSlots[i].get())
			{
				textureSlotIndex = i;
				break;
			}
		}

		if (textureSlotIndex == 0) {
			textureSlotIndex = (int)s_Data->textureSlotIndex;
			s_Data->textureSlots[s_Data->textureSlotIndex++] = texture;
		}

		for (int i = 0; i < 4; i++) {
			s_Data->quadVertexBufferPtr->position = transform * s_Data->QuadVertices[i];
			s_Data->quadVertexBufferPtr->color = color;
			s_Data->quadVertexBufferPtr->texCrood = s_Data->QuadTextureCrood[i];
			s_Data->quadVertexBufferPtr->texIndex = (GLint)textureSlotIndex;			
			s_Data->quadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;
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
			s_Data->quadVertexBufferPtr->position = transform * s_Data->QuadVertices[i];
			s_Data->quadVertexBufferPtr->color = color;
			s_Data->quadVertexBufferPtr->texCrood = s_Data->QuadTextureCrood[i];
			s_Data->quadVertexBufferPtr->texIndex = 0;
			s_Data->quadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;

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
			s_Data->quadVertexBufferPtr->position = transform * s_Data->QuadVertices[i];
			s_Data->quadVertexBufferPtr->color = color;
			s_Data->quadVertexBufferPtr->texCrood = s_Data->QuadTextureCrood[i];
			s_Data->quadVertexBufferPtr->texIndex = 0;
			s_Data->quadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;

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
		for (GLint i = 1; i < s_Data->textureSlotIndex; i++) {
			if (*texture.get() == *s_Data->textureSlots[i].get())
			{
				textureSlotIndex = i;
				break;
			}
		}

		if (textureSlotIndex == 0) {
			textureSlotIndex = (int)s_Data->textureSlotIndex;
			s_Data->textureSlots[s_Data->textureSlotIndex++] = texture;
		}


		glm::vec4 color = glm::vec4(1.0f);



		for (int i = 0; i < 4; i++) {
			s_Data->quadVertexBufferPtr->position = transform * s_Data->QuadVertices[i];
			s_Data->quadVertexBufferPtr->color = color;
			s_Data->quadVertexBufferPtr->texCrood = s_Data->QuadTextureCrood[i];
			s_Data->quadVertexBufferPtr->texIndex = (GLint)textureSlotIndex;
			s_Data->quadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;
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
		for (GLint i = 1; i < s_Data->textureSlotIndex; i++) {
			if (*texture.get() == *s_Data->textureSlots[i].get())
			{
				textureSlotIndex = i;
				break;
			}
		}

		if (textureSlotIndex == 0) {
			textureSlotIndex = (int)s_Data->textureSlotIndex;
			s_Data->textureSlots[s_Data->textureSlotIndex++] = texture;
		}

		for (int i = 0; i < 4; i++) {
			s_Data->quadVertexBufferPtr->position = transform * s_Data->QuadVertices[i];
			s_Data->quadVertexBufferPtr->color = color;
			s_Data->quadVertexBufferPtr->texCrood = s_Data->QuadTextureCrood[i];
			s_Data->quadVertexBufferPtr->texIndex = (GLint)textureSlotIndex;
			s_Data->quadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;
		m_RendererState.quadAmount += 1;
		m_RendererState.vertexAmount += 4;
		m_RendererState.indexAmount += 6;
	}
	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, const float thickness, const float fade, const int entityID)
	{
		for (int i = 0; i < 4; i++) {
			s_Data->circleVertexBuffePtr->worldPosition = transform * s_Data->QuadVertices[i];
			s_Data->circleVertexBuffePtr->localPosition = s_Data->QuadVertices[i] * 2.0f;
			s_Data->circleVertexBuffePtr->color = color;
			s_Data->circleVertexBuffePtr->thickness = thickness;
			s_Data->circleVertexBuffePtr->fade = fade;			
			s_Data->circleVertexBuffePtr++;
		}

		s_Data->circleIndexCount += 6;

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
	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int enitityID)
	{
		s_Data->lineVertexBuffePtr->position = p0;
		s_Data->lineVertexBuffePtr->color = color;		
		s_Data->lineVertexBuffePtr++;

		s_Data->lineVertexBuffePtr->position = p1;
		s_Data->lineVertexBuffePtr->color = color;		
		s_Data->lineVertexBuffePtr++;

		s_Data->lineIndexCount += 2;
	}
	void Renderer2D::DrawRect(const glm::mat4 transform, const glm::vec4& color, int enitityID)
	{
		glm::vec3 rectVertex[4];
		rectVertex[0] = transform * s_Data->QuadVertices[0];  //bottom left
		rectVertex[1] = transform * s_Data->QuadVertices[1];  //bottom right
		rectVertex[2] = transform * s_Data->QuadVertices[2];  //top left
		rectVertex[3] = transform * s_Data->QuadVertices[3];  //top right


		DrawLine(rectVertex[0], rectVertex[1], color, enitityID);
		DrawLine(rectVertex[0], rectVertex[2], color, enitityID);
		DrawLine(rectVertex[1], rectVertex[3], color, enitityID);
		DrawLine(rectVertex[2], rectVertex[3], color, enitityID);

	}
	Ref<Framebuffer> Renderer2D::GetFramebuffer()
	{
		return s_Data->frameBuffer;
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