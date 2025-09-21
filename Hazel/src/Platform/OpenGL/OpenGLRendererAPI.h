#pragma once

#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel {

	class OpenGLRendererAPI : public RendererAPI {
	public:
		OpenGLRendererAPI();
		~OpenGLRendererAPI();

		virtual void Init() override;
		
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		inline virtual void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count) override;		

		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		virtual void SetLineWidth(float width) override;

		virtual void SetDepthTest(bool value) override;
		virtual void SetDepthMask(bool value) override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void DrawFrame() override;
		virtual void BeginRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass) override;		
		virtual void EndRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass) override;		
		virtual void DrawIndexed(Ref<CommandBuffer> commandBuffer, const Ref<VertexArray>& vertexArray) override;
		virtual void DrawIndexed(Ref<CommandBuffer> commandBuffer, const Ref<VertexArray>& vertexArray, uint32_t count) override;

		virtual void SubmitMaterial(Ref<CommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material>) override;

	};
}