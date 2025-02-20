#include "hzpch.h"
#include "Platform/OpenGL/OpenGLEnvironment.h"

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel {



	//IBL
	OpenGLEnvironment::OpenGLEnvironment(std::string filepath)
	{
		if (!Environment::s_EquirectangularConversionShader) {
			s_EquirectangularConversionShader = ShaderLibrary::Load("assets/Shaders/EquirectangularToCubeMap.glsl");
		}
		Ref<Texture2D> envEquirect = TextureLibrary::Load(filepath);


		if (envEquirect->GetTextureFormat() != TextureFormat::Float16)
		{
			HZ_CORE_WARN("{0} is not HDR image! {0}", filepath);
			return;
		}

		const uint32_t cubemapSize = 2048;
		const uint32_t irradianceMapSize = 32;
		const uint32_t BRDFLUTSize = 512;

		Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
		m_EnvFiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);

		{
			envEquirect->Bind();
			s_EquirectangularConversionShader->Bind();
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


		if (!s_EnvFilteringShader) {
			s_EnvFilteringShader = ShaderLibrary::Load("assets/Shaders/EnvironmentMipFilter.glsl");
		}


		//TODO : Submit
		{
			s_EnvFilteringShader->Bind();
			envUnfiltered->Bind();
			const float deltaRoughness = 1.0f / glm::max((float)(m_EnvFiltered->GetMipmapCount() - 1.0f), 1.0f);
			for (int level = 1, size = cubemapSize / 2; level < m_EnvFiltered->GetMipmapCount(); level++, size / 2) {
				const GLuint numGroups = glm::max(1, size / 32);
				glBindImageTexture(0, m_EnvFiltered->GetRendererID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glProgramUniform1f(s_EnvFilteringShader->GetRendererID(), 0, level * deltaRoughness);
				glDispatchCompute(numGroups, numGroups, 6);
			}
		}

		//Environment Irradiance

		if (!s_EnvIrradianceShader) {
			s_EnvIrradianceShader = ShaderLibrary::Load("assets/Shaders/EnvironmentIrradiance.glsl");
		}


		m_IrradianceMap = TextureCube::Create(TextureFormat::Float16, irradianceMapSize, irradianceMapSize);

		{
			s_EnvIrradianceShader->Bind();
			m_EnvFiltered->Bind();
			glBindImageTexture(0, m_IrradianceMap->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(m_IrradianceMap->GetWidth() / 32, m_IrradianceMap->GetHeight() / 32, 6);
			glGenerateTextureMipmap(m_IrradianceMap->GetRendererID());
		}


		m_IsLoaded = true;


		//BRDFLUT
		if (!s_BRDFLUT || !s_BRDFLUT->IsLoaded()) {
			s_BRDFLUT = Texture2D::Create(TextureFormat::Float16, BRDFLUTSize, BRDFLUTSize);
			if (!s_GenerateBRDFLUTShader) {
				s_GenerateBRDFLUTShader = ShaderLibrary::Load("assets/Shaders/GenerateBRDFLUT.glsl");
			}
			//submit
			{
				s_GenerateBRDFLUTShader->Bind();
				glBindImageTexture(0, s_BRDFLUT->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(s_BRDFLUT->GetWidth() / 32, s_BRDFLUT->GetHeight() / 32, 1);
				s_BRDFLUT->SetIsLoaded(true);
			}

		}
	}

}