#pragma once


#include "Hazel/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Hazel {

	class RenderCommand {
	public:

		inline static void Init() {
			
			switch (RendererAPI::GetAPI())
			{
			case RendererAPI::API::OpenGL:
				s_RendererAPI = MakeScope<OpenGLRendererAPI>();
			case RendererAPI::API::Vulkan:
				s_RendererAPI = MakeScope<VulkanRendererAPI>();
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "Not Valid Render API!");
				return;
			}

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

		static void BeginFrame() { s_RendererAPI->BeginFrame(); }
		static void EndFrame() { s_RendererAPI->EndFrame(); }
		static void DrawFrame() { s_RendererAPI->DrawFrame(); }
		static void BeginRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass, uint32_t imageIndex) 	{
			s_RendererAPI->BeginRenderPass(commandBuffer, renderPass, imageIndex);
		}
		static void BindVertexBuffer(Ref<CommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer) { 
			s_RendererAPI->BindVertexBuffer(commandBuffer, vertexBuffer);
		}
		static void EndRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass) { 
			s_RendererAPI->EndRenderPass(commandBuffer, renderPass);
		}
		static void BindVertexArray(Ref<CommandBuffer> commandBuffer, Ref<VertexArray> vertexArray) { 
			s_RendererAPI->BindVertexArray(commandBuffer, vertexArray);
		}
		static void BindIndexBuffer(Ref<CommandBuffer> commandBuffer, Ref<IndexBuffer> indexBuffer) { 
			s_RendererAPI->BindIndexBuffer(commandBuffer, indexBuffer);
		}
		static void DrawIndexed(Ref<CommandBuffer> commandBuffer, uint32_t count) { 
			s_RendererAPI->DrawIndexed(commandBuffer, count);
		}

		static void  SubmitMaterial(Ref<CommandBuffer> commandBuffer, Ref<Material> material) {
			s_RendererAPI->SubmitMaterial(commandBuffer, material);
		}

	private:
		friend class Renderer;
		static Scope<RendererAPI> s_RendererAPI;
	};

}