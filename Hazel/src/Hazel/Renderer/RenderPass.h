#pragma once

#include "Hazel/Core/Core.h"




namespace Hazel {

	class Pipeline;

	class Framebuffer;

	struct RenderPassSpecification {
		Ref<Framebuffer> targetFrameBuffer = nullptr;
		Ref<Pipeline> pipeline = nullptr;

	};


	class RenderPass {
	public:
		RenderPass() = default;
		virtual ~RenderPass() = default;
		RenderPass(RenderPassSpecification spec);

		virtual RenderPassSpecification& GetSpecification();

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);


	protected:

		RenderPassSpecification m_Specification;
	};

}