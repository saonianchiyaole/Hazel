#pragma once
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Scene/Component.h"

namespace Hazel {

	class Framebuffer;
	class Texture2D;

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

		static void Resize(glm::vec2 size);

		static void BeginFrame();
		static void BeginScene(const OrthographicCamera& camera);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const Camera& camera);
		static void EndScene();

		static void Flush();

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, Ref<Texture2D> texture, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, int entityID);

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float angle, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float angle, const glm::vec4& color);

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, Handle<Texture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Handle<Texture2D>& texture);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float angle, Handle<Texture2D>& texture);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float angle, Handle<Texture2D>& texture);

		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, const float thickness, const float fade, const int entityID = -1);
		static void DrawSprite(const glm::mat4& transform, const SpriteComponent& sprite, const int entityID);

		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int enitityID = -1);
		static void DrawRect(const glm::mat4 transform, const glm::vec4& color, int enitityID = -1);
		
		static bool IsInCriticalSection();

		// todo remove this to renderer / sceneRenderer
		static Ref<Framebuffer> GetFramebuffer();
		static bool SetViewportSize(const glm::vec2& size);

		const static RendererState* GetState();
		static void ResetState();
	private:
		static RendererState m_RendererState;


	};
}