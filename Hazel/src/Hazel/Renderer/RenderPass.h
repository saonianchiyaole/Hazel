#pragma once

#include "Hazel/Core/Core.h"




namespace Hazel {

	class Pipeline;

	class Framebuffer;

	struct RenderPassSpecification {
		
		// todo : shoudn't be used, should be move to pipeline
		Ref<Framebuffer> targetFramebuffer;

		Ref<Pipeline> pipeline = nullptr;

	};


	class RenderPass {
	public:
		RenderPass() = default;
		virtual ~RenderPass() = default;
		RenderPass(RenderPassSpecification spec);
		
		RenderPassSpecification&	GetSpecification()		{ return m_Specification; }
		Ref<Framebuffer>			GetTargetFramebuffer()	{ return m_Specification.targetFramebuffer; }
		Ref<Pipeline>				GetPipeline()			{ return m_Specification.pipeline; }

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);


	protected:

		RenderPassSpecification m_Specification;
	};

}