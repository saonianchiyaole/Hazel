#include "hzpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include "stb_image.h"
#include "glad/glad.h"


namespace Hazel {
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
	{
		HZ_CORE_INFO("Open Texture filePath {0}", path);
		if (path.empty()) {
			HZ_CORE_WARN("This is Not Texture file!");
			return;
		}


		m_Path = path;

		stbi_set_flip_vertically_on_load(true); // ¼ÓÔØÍ¼ÏñÊ±·­×ªYÖá


		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		HZ_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;

		if (channels == 4) {
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
			m_Type = GL_UNSIGNED_BYTE;
		}
		else if (channels == 3) {
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
			m_Type = GL_UNSIGNED_BYTE;
		}
		else if (channels == 2) {
			m_InternalFormat = GL_RG8;
			m_DataFormat = GL_RG;
			m_Type = GL_UNSIGNED_BYTE;
		}

		HZ_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Image's format is not supported !");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTexParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, m_Type, data);

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
}