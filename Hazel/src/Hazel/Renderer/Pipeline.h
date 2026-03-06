#pragma once


#include "Hazel/Renderer/Buffer.h"

#include "vulkan/vulkan.h"

namespace Hazel {

	class Shader;
	class Framebuffer;


	enum class PrimitiveTopology {
		None = 0, TriangleList, LineList, PointList
	};


	struct PipelineSpecification {

		Ref<Shader> shader;

		BufferLayout bufferLayout;
		
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;
		
		uint32_t multiSampleCount;

		bool isWireframe = false;

		float lineWidth = 1.0f;
	};

	class Pipeline {

	public:

		Pipeline() = default;

		Pipeline(const PipelineSpecification& specification) : m_Specification(specification){}

		virtual ~Pipeline() = default; // Ìí¼ÓÐéÎö¹¹º¯Êý

		static Ref<Pipeline>	Create(const PipelineSpecification& specification);

		PipelineSpecification&	GetSpecification()		{ return m_Specification; }
		float					GetLineWidth()			{ return m_Specification.lineWidth; }

	private:

	protected:

		PipelineSpecification m_Specification;


	};



}