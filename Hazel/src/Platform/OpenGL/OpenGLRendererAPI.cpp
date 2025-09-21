#include "hzpch.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

#include "glad/glad.h"


namespace Hazel {



	OpenGLRendererAPI::OpenGLRendererAPI()
	{
	}

	OpenGLRendererAPI::~OpenGLRendererAPI()
	{

	}

	void OpenGLRendererAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	inline void OpenGLRendererAPI::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count)
	{

		//HZ_CORE_ASSERT(count, "Count can't equals 0!")
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}


	void OpenGLRendererAPI::SubmitMaterial(Ref<CommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material>)
	{

	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}
	void OpenGLRendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}
	void OpenGLRendererAPI::SetDepthTest(bool value)
	{
		if (value)
			glDisable(GL_DEPTH_TEST);
		else
			glEnable(GL_DEPTH_TEST);
	}
	void OpenGLRendererAPI::SetDepthMask(bool value)
	{
		glDepthMask(value);
	}

	void OpenGLRendererAPI::BeginFrame()
	{

	}

	void OpenGLRendererAPI::EndFrame()
	{

	}

	void OpenGLRendererAPI::DrawFrame()
	{

	}

	void OpenGLRendererAPI::BeginRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
	{

	}
	
	void OpenGLRendererAPI::EndRenderPass(Ref<CommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
	{

	}

	void OpenGLRendererAPI::DrawIndexed(Ref<CommandBuffer> commandBuffer, const Ref<VertexArray>& vertexArray)
	{

	}

	void OpenGLRendererAPI::DrawIndexed(Ref<CommandBuffer> commandBuffer, const Ref<VertexArray>& vertexArray, uint32_t count)
	{

	}

	

}