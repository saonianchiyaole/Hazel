#include "hzpch.h"
#include "Platform/OpenGL/OpenGLEnvironment.h"

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel {




	OpenGLEnvironment::OpenGLEnvironment(std::string filepath)
	{
		if (!Environment::equirectangularConversionShader) {
			equirectangularConversionShader = ShaderLibrary::Load("assets/Shaders/EquirectangularToCubeMap.glsl");
		}
		Ref<Texture2D> envEquirect = TextureLibrary::Load(filepath);

		const uint32_t cubemapSize = 2048;
		const uint32_t irradianceMapSize = 32;

		Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);

		envEquirect->Bind();
		equirectangularConversionShader->Bind();
		glBindImageTexture(0, envUnfiltered->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glDispatchCompute(cubemapSize / 32, cubemapSize / 32, 6);
		glGenerateTextureMipmap(envUnfiltered->GetRendererID());

		envUnfiltered->m_Path = filepath;
		envUnfiltered->SetIsLoaded(true);

		m_EnvFiltered = envUnfiltered;

	}

}