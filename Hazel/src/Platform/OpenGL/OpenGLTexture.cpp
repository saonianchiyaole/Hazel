#include "hzpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include "stb_image.h"
#include "glad/glad.h"


#define POSITIVE_X 0;
#define NEGATIVE_X 1;
#define POSITIVE_Y 2;
#define NEGATIVE_Y 3;
#define POSITIVE_Z 4;
#define NEGATIVE_Z 5;

namespace Hazel {
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
	{
		HZ_CORE_INFO("Open Texture filePath {0}", path);
		std::filesystem::path checkpath = path;
		if (checkpath.extension().string() != ".jpg" && checkpath.extension().string() != ".png") {
			HZ_CORE_WARN("This is Not Texture file!");
			return;
		}


		m_Path = path;

		stbi_set_flip_vertically_on_load(true); // ¼ÓÔØÍ¼ÏñÊ±·­×ªYÖá


		if (stbi_is_hdr(path.c_str())) {
			m_IsHDR = true;
		}


		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		HZ_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;

		if (channels == 4) {
			m_TextureFormat = TextureFormat::RGBA;
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
			m_DataType = GL_UNSIGNED_BYTE;
		}
		else if (channels == 3 && !m_IsHDR) {
			m_TextureFormat = TextureFormat::RGB;

			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
			m_DataType = GL_UNSIGNED_BYTE;

		}
		else if (channels == 3 && m_IsHDR) {
			m_TextureFormat = TextureFormat::Float16;

			m_InternalFormat = GL_RGB16F;
			m_DataFormat = GL_RGB;
			m_DataType = GL_FLOAT;
		}
		else if (channels == 2) {
			m_TextureFormat = TextureFormat::RG;
			m_InternalFormat = GL_RG8;
			m_DataFormat = GL_RG;
			m_DataType = GL_UNSIGNED_BYTE;
		}

		HZ_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Image's format is not supported !");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTexParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, m_DataType, data);

		m_Loaded = true;

		stbi_image_free(data);
	}
	OpenGLTexture2D::OpenGLTexture2D(const uint32_t width, const uint32_t height)
	{
		m_Width = width;
		m_Height = height;

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTexParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	}
	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLTexture2D::SetData(const void* data, const uint32_t size)
	{
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		HZ_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
		m_Loaded = true;
	}


	void OpenGLTexture2D::Bind(uint32_t slot) const {
		glBindTextureUnit(slot, m_RendererID);
	}
	const uint32_t OpenGLTexture2D::GetRendererID()
	{
		return m_RendererID;
	}

	OpenGLTextureCube::OpenGLTextureCube()
	{
		glCreateBuffers(1, &m_RendererID);
		m_Textures.resize(6);
		for (int i = 0; i < 6; i++) {
			m_Textures[i] = nullptr;
		}
	}

	//LDR TextureCube
	OpenGLTextureCube::OpenGLTextureCube(std::vector<Ref<Texture2D>> textures)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);


		for (size_t i = 0; i < textures.size(); i++) {


			glBindTexture(GL_TEXTURE_2D, textures[i]->GetRendererID());

			uint32_t size = textures[i]->m_Width * textures[i]->m_Height;

			if (textures[i]->m_DataType == GL_UNSIGNED_BYTE)
				size *= 1 * 3;
			else if (textures[i]->m_DataType == GL_FLOAT)
				size *= 2 * 3;

			void* data = malloc(size);
			glGetTexImage(GL_TEXTURE_2D, 0, textures[i]->m_DataFormat, textures[i]->m_DataType, data);

			glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, textures[i]->m_InternalFormat, textures[i]->m_Width,
				textures[i]->m_Height, 0, textures[i]->m_DataFormat, textures[i]->m_DataType, data);

			free(data);

			m_Textures.push_back(textures[i]);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		m_IsLoaded = true;
	}
	uint32_t OpenGLTextureCube::GetRendererID()
	{
		return m_RendererID;
	}
	void OpenGLTextureCube::SetTexture(Ref<Texture2D> texture, uint32_t slot)
	{

		if (!texture->IsLoaded() || texture.get() == m_Textures[slot].get() || (slot > 5 || slot < 0)) {
			return;
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		m_Textures[slot] = texture;

		glBindTexture(GL_TEXTURE_2D, texture->GetRendererID());

		uint32_t size = texture->m_Width * texture->m_Height;

		if (texture->m_DataType == GL_UNSIGNED_BYTE)
			size *= 1 * 3;
		else if (texture->m_DataType == GL_FLOAT)
			size *= 2 * 3;

		void* data = malloc(size);
		glGetTexImage(GL_TEXTURE_2D, 0, texture->m_DataFormat, texture->m_DataType, data);

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + slot, 0, texture->m_InternalFormat, texture->m_Width,
			texture->m_Height, 0, texture->m_DataFormat, texture->m_DataType, data);

		free(data);

		m_IsLoaded = true;
		for (size_t i = 0; i < 6; i++) {
			if (!texture) {
				m_IsLoaded = false;
				break;
			}
		}

	}
}