#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/ByteKey.h"

#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Pipeline.h"

namespace Hazel {

	class Pipeline;
	class UniformBufferSet;
	class UniformBuffer;
	class Texture2D;
	class Framebuffer;

	struct RenderPassSpecification {

		// todo : shoudn't be used, should be move to pipeline

		std::vector<AttachmentSpecification> attachmentSpecs;
		
		std::shared_ptr<Pipeline> pipeline = nullptr;

		std::shared_ptr<Framebuffer> framebuffer = nullptr;

	};

	
	class RenderPass {
	public:		
		RenderPass() = default;
		RenderPass(const RenderPassSpecification& spec);
		virtual ~RenderPass() = default;		
		
		RenderPassSpecification&	GetSpecification()		{ return m_Specification; }	

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);



		// Render pass input can't not be set by direct value
		virtual bool SetData		(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) = 0;
		virtual bool SetData		(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) = 0;
		virtual bool SetData		(const std::string& name, Ref<Texture2D> texture, uint32_t index = 0) = 0;

		virtual void SetFramebuffer		(Ref<Framebuffer> framebuffer)		{ m_Specification.framebuffer = framebuffer; }
		virtual void SetPipeline		(Ref<Pipeline> pipeline)			{ m_Specification.pipeline = pipeline; }
		virtual void SetPipelineState	(const PipelineSpecification& spec)	{ m_Specification.pipeline = MakeRef<Pipeline>(spec); };

		// submit data to descriptor
		virtual void Submit() = 0;

	protected:

		RenderPassSpecification m_Specification;
	};

}