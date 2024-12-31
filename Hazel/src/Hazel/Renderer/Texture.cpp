#include "hzpch.h"
#include "Hazel/Renderer/Texture.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#include "Hazel/Renderer/Renderer.h"

namespace Hazel {

	/*Ref<Texture2D> Texture2D::Create(const std::string& path) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLTexture2D>(path);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}*/

	Ref<Texture2D> Texture2D::Create(const uint32_t width, const uint32_t height)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLTexture2D>(width, height);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(std::filesystem::path path)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLTexture2D>(path.string());

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	void Texture2D::SetType(TextureType type)
	{
		m_Type = type;
	}

	TextureType Texture2D::GetType()
	{
		return m_Type;
	}


	uint32_t Texture2D::GetWidth() const
	{
		return m_Width;
	}
	uint32_t Texture2D::GetHeight() const
	{
		return m_Height;
	}

	std::string Texture2D::GetPath() const
	{
		return m_Path;
	}

}