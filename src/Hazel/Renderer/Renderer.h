#pragma once

#include "Hazel/Renderer/RenderCommand.h"

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Camera.h"

namespace Hazel {
	
	class Renderer {
	public:

		static void BeginScene(Camera& camera);

		static void EndScene();

		static void Submit(const Ref<VertexArray>& vertexArray, Ref<Shader>& shader, const glm::mat4& transform = glm::mat4(1.0f));

		static void Init();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void OnWindowResize(uint32_t width, uint32_t height);

	private:
		struct SceneData {
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewProjectionMatrix;
		};
		static SceneData* m_SceneData;
	};

}