#include "hzpch.h"
#include "RenderPass.h"

#include "Hazel/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/OpenGL/OpenGLRenderPass.h"

#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel {
	
	

	namespace Utils {

		ByteKey GetRenderPassByteKey(const RenderPassInfo& spec) {

			auto attachmentSpec = spec.attachmentInfos;

			ByteKey byteKey;
			for (auto& spec : attachmentSpec) {
				byteKey.AddBytes(spec.format);
				byteKey.AddBytes(spec.isClearColor);
				byteKey.AddBytes(spec.clearValue.color);
			}

			return byteKey;

		}

	}

	RenderPass::RenderPass(const RenderPassInfo& spec) : m_Info(spec)
	{
		
	}



	Ref<RenderPass> RenderPass::Create(const RenderPassInfo& spec)
	{

		switch (RendererAPI::GetAPI()) {

		case RendererAPI::API::Vulkan:
			return MakeRef<RenderPass>(spec);
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLRenderPass>(spec);
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "Invalid Renderer API!");
		}
		
	}

}