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
		
		Ref<Framebuffer> targetFramebuffer;

		PrimitiveTopology topology = PrimitiveTopology::TriangleList;
		
		bool isWireframe = false;
		float lineWidth = 1.0f;
	};

	class Pipeline {

	public:

		virtual ~Pipeline() = default; // Ìí¼ÓÐéÎö¹¹º¯Êý

		static Ref<Pipeline> Create(PipelineSpecification specification);
		Ref<Framebuffer>	GetTargetFramebuffer()	{ return m_Specification.targetFramebuffer; }
		float				GetLineWidth()			{ return m_Specification.lineWidth; }

	private:

	protected:

		PipelineSpecification m_Specification;


	};



}