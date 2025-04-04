#pragma once
#include "glm/glm.hpp"
#include "Hazel/Renderer/VertexArray.h"

namespace Hazel {

	class RendererAPI {
	public:
		enum class API {
			None = 0,
			OpenGL = 1
		};
	public:
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		inline virtual void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void Init() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count) = 0;
		
		virtual void DrawElementsBaseVertex(uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) = 0;

		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void SetLineWidth(float width) = 0;

		virtual void SetDepthTest(bool value) = 0;
		virtual void SetDepthMask(bool value) = 0;

		static 	API GetAPI() { return s_API; }

		
	private:
		static API s_API;
	};



}