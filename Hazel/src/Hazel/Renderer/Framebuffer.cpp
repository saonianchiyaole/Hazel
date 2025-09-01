#include "hzpch.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"

namespace Hazel {


	namespace Utils {

		bool IsDepthFormat(TextureFormat format) {

			return format == TextureFormat::DEPTH24STENCIL8;

		}

	}

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLFramebuffer>(spec);
		case RendererAPI::API::Vulkan:
			return MakeRef<VulkanFramebuffer>(spec);
		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}
	
}