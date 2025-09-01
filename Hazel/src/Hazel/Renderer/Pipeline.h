#pragma once


#include "Hazel/Renderer/Buffer.h"

#include "vulkan/vulkan.h"

namespace Hazel {

	class Shader;
	class Framebuffer;

	struct PipelineSpecification {

		Ref<Shader> shader;

		BufferLayout bufferLayout;				
		
		Ref<Framebuffer> targetFramebuffer;

	};

	class Pipeline {

	public:

		virtual ~Pipeline() = default; // Ìí¼ÓÐéÎö¹¹º¯Êý

		static Ref<Pipeline> Create(PipelineSpecification specification);
		Ref<Framebuffer> GetTargetFramebuffer() { return m_Specification.targetFramebuffer; }

	private:

	protected:

		PipelineSpecification m_Specification;


	};



}