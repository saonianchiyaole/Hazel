#include "hzpch.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel {
	

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {

		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLVertexBuffer>(vertices, size);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(void* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLVertexBuffer>(vertices, size);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}
	
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) {

		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLVertexBuffer>(size);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}


	VertexBuffer::~VertexBuffer() {

	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLIndexBuffer>(indices, count);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(void* indices, uint32_t count)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLIndexBuffer>(indices, count);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	IndexBuffer::~IndexBuffer() {

	}

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLUniformBuffer>(size, binding);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

}