#pragma once


#include "Hazel/Renderer/Texture.h"

namespace Hazel {
	class OpenGLTexture : public Texture {

	};


	class OpenGLTexture2D : public Texture2D {
	public:
		~OpenGLTexture2D();
		OpenGLTexture2D() = default;
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(const uint32_t width, const uint32_t height);

		virtual void SetData(const void* data, const uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual const uint32_t GetRendererID() override;
	
		virtual bool operator ==(Texture& other) const override {
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

		friend class OpenGLTextureCube;
	private:
		uint32_t m_RendererID;
	};


	class OpenGLTextureCube : public TextureCube{
	public:
		OpenGLTextureCube();
		OpenGLTextureCube(std::vector<Ref<Texture2D>> textures);
		OpenGLTextureCube(const std::string& path);
		virtual uint32_t GetRendererID() override;
		virtual void SetTexture(Ref<Texture2D> texture, uint32_t slot) override;
	private:
		uint32_t m_RendererID;
	};
}