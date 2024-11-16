#pragma once


#include "Hazel/Renderer/Texture.h"

namespace Hazel {
	class OpenGLTexture : public Texture {

	};


	class OpenGLTexture2D : public Texture2D {
	public:
		~OpenGLTexture2D();
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(const uint32_t width, const uint32_t height);

		virtual void SetData(const void* data, const uint32_t size) override;

		virtual uint32_t GetWidth() const override;
		virtual uint32_t GetHeight() const override;
		virtual void Bind(uint32_t slot = 0) const override;

		virtual const uint32_t GetRendererID() override;

		virtual bool operator ==(Texture& other) const override {
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		unsigned int m_InternalFormat = 0;
		unsigned int m_DataFormat = 0;
		uint32_t m_RendererID;
	};
}