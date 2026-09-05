#pragma once


#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Shader.h"

#include "vulkan/vulkan.h"


namespace Hazel {

		
	enum class PrimitiveTopology {
		None = 0, TriangleList, LineList, PointList
	};


	struct PipelineInfo {

		Handle<Shader> shader;

		BufferLayout bufferLayout;
		
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;
		
		uint32_t multiSampleCount;

		bool isWireframe = false;

		float lineWidth = 1.0f;
	};

	class Pipeline {

	public:

		Pipeline() = default;

		Pipeline(const PipelineInfo& info) : m_Info(info){}

		virtual ~Pipeline() = default; // Ìí¼ÓÐéÎö¹¹º¯Êý

		static Ref<Pipeline>	Create(const PipelineInfo& specification);

		PipelineInfo&	GetSpecification()		{ return m_Info; }
		float					GetLineWidth()			{ return m_Info.lineWidth; }

	private:

	protected:

		PipelineInfo m_Info;


	};



}