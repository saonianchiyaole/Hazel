#include "hzpch.h"
#include "Hazel/Renderer/VertexArray.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"


namespace Hazel {


	Ref<VertexArray> VertexArray::Create() {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLVertexArray>();

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

}