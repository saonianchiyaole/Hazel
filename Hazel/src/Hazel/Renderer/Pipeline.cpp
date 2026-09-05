#include "hzpch.h"


#include "Hazel/Renderer/Pipeline.h"

#include "Hazel/Renderer/RendererAPI.h"


#include "Platform/Vulkan/VulkanPipeline.h"

namespace Hazel {





	Ref<Pipeline> Pipeline::Create(const PipelineInfo& info)
	{
		
		switch (RendererAPI::GetAPI()) {


		case RendererAPI::API::Vulkan:
			return MakeRef<Pipeline>(info);
		case RendererAPI::API::OpenGL:
			return MakeRef<Pipeline>(info); // OpenGL implementation not provided in this example
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "Invalid Renderer API!");


		}



	}





}

