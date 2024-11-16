#pragma once



#include "hzpch.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "glm/glm.hpp"

namespace Hazel {

	struct FramebufferSpecification {
		uint32_t width;
		uint32_t height;
		uint32_t samples = 1;
		bool SwapChainTarget = false;
	};

	class Framebuffer {
	public:

		virtual const FramebufferSpecification& GetSpecification() = 0;
		virtual void Invalidate() = 0;
		virtual void Resize(const FramebufferSpecification& spec) = 0;
		virtual void Resize(const glm::vec2 size) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual const uint32_t GetColorAttachment() = 0;
		virtual const uint32_t GetDpethAttachment() = 0;


		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	protected:

	};

}