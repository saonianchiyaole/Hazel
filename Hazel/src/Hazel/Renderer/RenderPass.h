#pragma once

#include "Hazel/Core/Core.h"

namespace Hazel {
	

	class Framebuffer;

	struct RenderPassSpec {
		Ref<Framebuffer> targetFrameBuffer;
	};


	class RenderPass {
	public:
		RenderPass() = default;
		RenderPass(RenderPassSpec spec);

		virtual RenderPassSpec GetSpecification();

		static Ref<RenderPass> Create(RenderPassSpec spec);
	private:
		RenderPassSpec m_Specification;
	};

}