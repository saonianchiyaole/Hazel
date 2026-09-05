#include "hzpch.h"
#include "Hazel/Renderer/VertexArray.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/Vulkan/VulkanVertexArray.h"

namespace Hazel {

	namespace {

		class VertexArrayResource : public VertexArray {
		public:
			void Bind() const override {}
			void Unbind() const override {}
			uint32_t GetRendererID() const override { return 0; }
		};

	}


	Ref<VertexArray> VertexArray::Create() {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLVertexArray>();
		case RendererAPI::API::Vulkan:
			return MakeRef<VertexArrayResource>();
		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

}
