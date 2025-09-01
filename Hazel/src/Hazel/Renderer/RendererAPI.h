#pragma once
#include "glm/glm.hpp"
#include "Hazel/Renderer/VertexArray.h"

namespace Hazel {

	class CommandBuffer;
	class RenderPass;
	class VertexBuffer;
	class IndexBuffer;
	class Material;
	class Pipeline;

	class RendererAPI {
	public:
		enum class API {
			None = 0,
			OpenGL = 1,
			Vulkan = 2
		};
	public:
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		inline virtual void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void Init() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count) = 0;
				
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void SetLineWidth(float width) = 0;

		virtual void SetDepthTest(bool value) = 0;
		virtual void SetDepthMask(bool value) = 0;


		// OpenGL Don't need this, just write empty functions in OpenGLRendererAPI
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void DrawFrame() = 0;		
		virtual void BeginRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass) = 0;
		virtual void BindVertexBuffer(Ref<CommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer) = 0;
		virtual void EndRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass) = 0;
		virtual void BindVertexArray(Ref<CommandBuffer> commandBuffer, Ref<VertexArray> vertexArray) = 0;
		virtual void BindIndexBuffer(Ref<CommandBuffer> commandBuffer, Ref<IndexBuffer> indexBuffer) = 0;		
		virtual void DrawIndexed(Ref<CommandBuffer> commandBuffer, uint32_t count) = 0;

		virtual void SubmitMaterial(Ref<CommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material) = 0;

		static 	API GetAPI() { return s_API; }

		
	private:
		static API s_API;
	};



}