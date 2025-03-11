#pragma once


#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel {

	class RenderCommand {
	public:

		inline static void Init() {
			s_RendererAPI->Init();
		}

		inline static void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
			s_RendererAPI->SetViewPort(x, y, width, height);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
			vertexArray->Bind();
			s_RendererAPI->DrawIndexed(vertexArray);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count) {
			s_RendererAPI->DrawIndexed(vertexArray, count);
		}
		static void DrawElementsBaseVertex(uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) {
			s_RendererAPI->DrawElementsBaseVertex(indexCount, baseIndex, baseVertex);
		}

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}
		static void SetLineWidth(float width) {
			s_RendererAPI->SetLineWidth(width);
		}
		static void SetDepthTest(bool value) {
			s_RendererAPI->SetDepthTest(value);
		}
		static void SetDepthMask(bool value) {
			s_RendererAPI->SetDepthMask(value);
		}
		static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }
		static void Clear() { s_RendererAPI->Clear(); }

	private:
		static RendererAPI* s_RendererAPI;
	};

}