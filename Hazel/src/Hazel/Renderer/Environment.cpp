#include "hzpch.h"

#include "Hazel/Renderer/Environment.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLEnvironment.h"

namespace Hazel {

	Ref<Shader> Environment::equirectangularConversionShader = nullptr;
	Ref<Shader> Environment::envFilteringShader = nullptr;
	Ref<Shader> Environment::envIrradianceShader = nullptr;

	Ref<Environment> Environment::Create(std::string filepath)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLEnvironment>(filepath);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

}