#include "hzpch.h"
#include "Hazel/Renderer/Texture.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#include "Hazel/Renderer/Renderer.h"

namespace Hazel {

	namespace Utils {
		uint32_t Hazel::Utils::CalculateMipmapCount(uint32_t width, uint32_t height)
		{
			uint32_t levels = 1;
			while ((width | height) >> levels)
				levels++;

			return levels;
		}

		template<typename T>
		uint32_t GetSizeOfTextureType() {

			if(std::issub)
			switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
			case RendererAPI::API::OpenGL:
				return sizeof(OpenGLTexture2D);
			}
			HZ_CORE_ASSERT(false, "Can't recognize the API!")
				return nullptr;
		}
	}



	std::unordered_map<std::string, std::shared_ptr<Hazel::Texture2D>> TextureLibrary::m_Textures;

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

	Ref<Texture2D> Texture2D::Create(TextureFormat format, const uint32_t width, const uint32_t height)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLTexture2D>(format, width, height);

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

	Ref<Texture2D> Texture2D::PreCreate()
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLTexture2D>();

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Texture2D* Texture2D::PreCreateNakedPointer()
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return new OpenGLTexture2D();

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

	uint32_t Texture2D::GetSlot()
	{
		return m_Slot;
	}

	void Texture2D::SetSlot(uint32_t slot)
	{
		m_Slot = slot;
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

	void TextureLibrary::Add(const Ref<Texture2D> texture)
	{
		auto& path = texture->GetPath();
		if (Exists(path))
		{
			HZ_CORE_ERROR("This Texture : {} already exist", path);
			return;
		}
		m_Textures[path] = texture;
	}

	Ref<Texture2D> TextureLibrary::Load(const std::string& path)
	{
		if (Exists(path))
		{
			HZ_CORE_ERROR("This Texture : {} already exist", path);
			return m_Textures[path];
		}

		Ref<Texture2D> texture = Texture2D::Create(path);

		Add(texture);
		return texture;
	}


	bool TextureLibrary::Exists(const std::string& path)
	{
		return m_Textures.find(path) != m_Textures.end();
	}

	Ref<Texture2D> TextureLibrary::Get(const std::string& path)
	{
		return m_Textures[path];
	}

	Ref<TextureCube> TextureCube::Create(std::vector<Ref<Texture2D>> textures)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLTextureCube>(textures);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const std::string& path)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLTextureCube>(path);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(TextureFormat format, const uint32_t width, const uint32_t height)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLTextureCube>(format, width, height);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<TextureCube> TextureCube::Create()
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLTextureCube>();

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	uint32_t TextureCube::GetWidth() const
	{
		return m_Width;
	}

	uint32_t TextureCube::GetHeight() const
	{
		return m_Height;
	}

	std::string TextureCube::GetPath() const
	{
		return m_Path;
	}

	uint32_t TextureCube::GetMipmapCount()
	{
		return Utils::CalculateMipmapCount(m_Width, m_Height);
	}

	std::vector<Ref<Texture2D>> TextureCube::GetTexutres()
	{
		return m_Textures;
	}

	bool TextureCube::IsLoaded()
	{
		return m_IsLoaded;
	}

	bool TextureCube::IsHDR()
	{
		return m_IsHDR;
	}

}

