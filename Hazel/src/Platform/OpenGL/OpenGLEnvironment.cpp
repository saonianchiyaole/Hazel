#include "hzpch.h"
#include "Platform/OpenGL/OpenGLEnvironment.h"

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel {



	//IBL
	OpenGLEnvironment::OpenGLEnvironment(std::string filepath)
	{
		if (!Environment::equirectangularConversionShader) {
			equirectangularConversionShader = ShaderLibrary::Load("assets/Shaders/EquirectangularToCubeMap.glsl");
		}
		Ref<Texture2D> envEquirect = TextureLibrary::Load(filepath);


		if (envEquirect->GetTextureFormat() != TextureFormat::Float16)
		{
			HZ_CORE_WARN("{0} is not HDR image! {0}", filepath);
			return;
		}

		const uint32_t cubemapSize = 2048;
		const uint32_t irradianceMapSize = 32;

		Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
		m_EnvFiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);

		{
			envEquirect->Bind();
			equirectangularConversionShader->Bind();
			glBindImageTexture(0, envUnfiltered->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(cubemapSize / 32, cubemapSize / 32, 6);
			glGenerateTextureMipmap(envUnfiltered->GetRendererID());
		}

		//envUnfiltered->m_Path = filepath;
		//envUnfiltered->SetIsLoaded(true);


		//Environment Prefilter
		m_EnvFiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);

		glCopyImageSubData(envUnfiltered->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
			m_EnvFiltered->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
			m_EnvFiltered->GetWidth(), m_EnvFiltered->GetHeight(), 6);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "OpenGL Error: " << error << std::endl;
		}

		m_EnvFiltered->m_Path = filepath;
		m_EnvFiltered->SetIsLoaded(true);
		m_EnvFiltered->m_IsHDR = true;

		
		if (!envFilteringShader) {
			envFilteringShader = ShaderLibrary::Load("assets/Shaders/EnvironmentMipFilter.glsl");
		}

	
		//TODO : Submit
		{
			envFilteringShader->Bind();
			envUnfiltered->Bind();
			const float deltaRoughness = 1.0f / glm::max((float)(m_EnvFiltered->GetMipmapCount() - 1.0f), 1.0f);
			for (int level = 1, size = cubemapSize / 2; level < m_EnvFiltered->GetMipmapCount(); level++, size / 2) {
				const GLuint numGroups = glm::max(1, size / 32);
				glBindImageTexture(0, m_EnvFiltered->GetRendererID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glProgramUniform1f(envFilteringShader->GetRendererID(), 0, level * deltaRoughness);
				glDispatchCompute(numGroups, numGroups, 6);
			}
		}

		//Environment Irradiance

		if (!envIrradianceShader) {
			envIrradianceShader = ShaderLibrary::Load("assets/Shaders/EnvironmentIrradiance.glsl");
		}


		m_IrradianceMap = TextureCube::Create(TextureFormat::Float16, irradianceMapSize, irradianceMapSize);

		{
			envIrradianceShader->Bind();
			m_EnvFiltered->Bind();
			glBindImageTexture(0, m_IrradianceMap->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(m_IrradianceMap->GetWidth() / 32, m_IrradianceMap->GetHeight() / 32, 6);
			glGenerateTextureMipmap(m_IrradianceMap->GetRendererID());
		}


		m_IsLoaded = true;

	}

}