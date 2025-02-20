#include "hzpch.h"

#include "Hazel/Renderer/Environment.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLEnvironment.h"

namespace Hazel {

	Ref<Shader> Environment::s_EquirectangularConversionShader = nullptr;
	Ref<Shader> Environment::s_EnvFilteringShader = nullptr;
	Ref<Shader> Environment::s_EnvIrradianceShader = nullptr;
	Ref<Shader> Environment::s_GenerateBRDFLUTShader = nullptr;
	Ref<Texture2D> Environment::s_BRDFLUT = nullptr;

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