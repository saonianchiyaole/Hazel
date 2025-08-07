#pragma once


#include "Hazel/Renderer/Buffer.h"

#include "vulkan/vulkan.h"

namespace Hazel {

	class Shader;

	struct PipelineSpecification {

		Ref<Shader> shader;

		BufferLayout bufferLayout;				

	};

	class Pipeline {

	public:

		virtual ~Pipeline() = default; // Ìí¼ÓÐéÎö¹¹º¯Êý

		static Ref<Pipeline> Create(PipelineSpecification specification);


	private:

	protected:

		PipelineSpecification m_Specification;


	};



}