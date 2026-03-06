#include "hzpch.h"


#include "Hazel/Renderer/Pipeline.h"

#include "Hazel/Renderer/RendererAPI.h"


#include "Platform/Vulkan/VulkanPipeline.h"

namespace Hazel {





	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specification)
	{
		
		switch (RendererAPI::GetAPI()) {


		case RendererAPI::API::Vulkan:
			return MakeRef<VulkanPipeline>(specification);			
		case RendererAPI::API::OpenGL:
			return MakeRef<Pipeline>(specification); // OpenGL implementation not provided in this example
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "Invalid Renderer API!");


		}



	}





}

