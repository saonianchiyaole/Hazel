#pragma once
#include "Hazel/Renderer/OrthographicCamera.h"

#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	class Renderer2D {
	public:
		struct RendererState {
			int quadAmount;
			int vertexAmount;
			int indexAmount;
			int drawCall;
			RendererState() {
				quadAmount = 0;
				vertexAmount = 0;
				indexAmount = 0;
				drawCall = 0;
			}
			
		};
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void Flush();




		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, Ref<Texture2D>& texture);

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float angle, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float angle, const glm::vec4& color);

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<Texture2D>& texture);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float angle, Ref<Texture2D>& texture);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float angle, Ref<Texture2D>& texture);

		const static RendererState* GetState();
		static void ResetState();
	private:
		static RendererState m_RendererState;


	};
}