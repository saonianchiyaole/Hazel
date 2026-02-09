#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/ByteKey.h"

namespace Hazel {

	class Pipeline;
	class UniformBufferSet;
	class UniformBuffer;
	class Texture2D;
	class Framebuffer;

	struct RenderPassSpecification {

		// todo : shoudn't be used, should be move to pipeline


		bool isClearColor;

		glm::vec4 clearColor;

		Ref<Framebuffer> targetFramebuffer;

		Ref<Pipeline> pipeline = nullptr;

	};

	namespace Utils {


		ByteKey GetRenderPassByteKey(const RenderPassSpecification& spec);

	}

	
	class RenderPass {
	public:		
		RenderPass() = default;
		RenderPass(const RenderPassSpecification& spec);
		virtual ~RenderPass() = default;		
		
		RenderPassSpecification&	GetSpecification()		{ return m_Specification; }
		Ref<Framebuffer>			GetTargetFramebuffer()	{ return m_Specification.targetFramebuffer; }
		Ref<Pipeline>				GetPipeline()			{ return m_Specification.pipeline; }

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);


		// Render pass input can't not be set by direct value
		virtual bool SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) = 0;
		virtual bool SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) = 0;
		virtual bool SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index = 0) = 0;

		// submit data to descriptor
		virtual void Submit() = 0;

	protected:

		RenderPassSpecification m_Specification;
	};

}