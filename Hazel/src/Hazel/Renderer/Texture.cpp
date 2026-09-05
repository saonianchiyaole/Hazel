#include "hzpch.h"
#include "Hazel/Renderer/Texture.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Hazel/Renderer/Renderer.h"

#include "stb_image.h"
#include <fstream>
#include <unordered_set>

namespace Hazel {
	
	namespace Utils {
		uint32_t Hazel::Utils::CalculateMipmapCount(uint32_t width, uint32_t height)
		{
			uint32_t levels = 1;
			while ((width | height) >> levels)
				levels++;

			return levels;
		}

		uint32_t GetPixelFormatChannelCount(PixelFormat format)
		{
			switch (format) {
			case PixelFormat::R: return 1;
			case PixelFormat::RG: return 2;
			case PixelFormat::RGB: return 3;
			case PixelFormat::RGBA: return 4;
			case PixelFormat::Float16: return 4;
			case PixelFormat::DEPTH24STENCIL8: return 1;
			case PixelFormat::None:
				break;
			}

			HZ_CORE_ASSERT(false, "Unknown pixel format!");
			return 0;
		}

		uint32_t GetPixelFormatSourceBytesPerChannel(PixelFormat format)
		{
			return format == PixelFormat::Float16 ? sizeof(float) : sizeof(uint8_t);
		}

		bool LoadTextureDataFromFile(const std::filesystem::path& path, TextureInfo& textureInfo, std::vector<uint8_t>& data)
		{
			if (!std::filesystem::exists(path)) {
				HZ_CORE_WARN("Texture file does not exist: {0}", path.string());
				return false;
			}

			static const std::unordered_set<std::string> supportedExtensions = {
				".jpg", ".jpeg", ".png", ".bmp", ".gif", ".hdr", ".tga"
			};

			if (supportedExtensions.find(path.extension().string()) == supportedExtensions.end()) {
				HZ_CORE_WARN("This is not a supported texture file: {0}", path.string());
				return false;
			}

			stbi_set_flip_vertically_on_load(true);

			int width = 0;
			int height = 0;
			int channels = 0;
			const std::string pathString = path.string();
			const bool isHDR = stbi_is_hdr(pathString.c_str());

			if (isHDR) {
				float* pixels = stbi_loadf(pathString.c_str(), &width, &height, &channels, 4);
				HZ_CORE_ASSERT(pixels, "Failed to load HDR image!");
				channels = 4;
				textureInfo.format = PixelFormat::Float16;
				textureInfo.width = static_cast<uint32_t>(width);
				textureInfo.height = static_cast<uint32_t>(height);
				textureInfo.depth = 1;
				textureInfo.usage = TextureUsage::Texture;

				const size_t size = static_cast<size_t>(width) * height * channels * sizeof(float);
				const uint8_t* begin = reinterpret_cast<const uint8_t*>(pixels);
				data.assign(begin, begin + size);
				stbi_image_free(pixels);
				return true;
			}

			stbi_uc* pixels = stbi_load(pathString.c_str(), &width, &height, &channels, 0);
			HZ_CORE_ASSERT(pixels, "Failed to load image!");

			switch (channels) {
			case 1: textureInfo.format = PixelFormat::R; break;
			case 2: textureInfo.format = PixelFormat::RG; break;
			case 3: textureInfo.format = PixelFormat::RGB; break;
			case 4: textureInfo.format = PixelFormat::RGBA; break;
			default:
				stbi_image_free(pixels);
				HZ_CORE_ASSERT(false, "Unsupported texture channel count!");
				return false;
			}

			textureInfo.width = static_cast<uint32_t>(width);
			textureInfo.height = static_cast<uint32_t>(height);
			textureInfo.depth = 1;
			textureInfo.usage = TextureUsage::Texture;

			const size_t size = static_cast<size_t>(width) * height * channels * sizeof(uint8_t);
			data.assign(pixels, pixels + size);
			stbi_image_free(pixels);
			return true;
		}
		

		template<typename T>
		uint32_t GetSizeOfTextureType() {
			return sizeof(T);
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

	Ref<Texture2D> Texture2D::Create(TextureInfo textureInfo, std::vector<uint8_t> data)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLTexture2D>(textureInfo, std::move(data));
		case RendererAPI::API::Vulkan:
			return MakeRef<Texture2D>(textureInfo, std::move(data));

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(std::filesystem::path path)
	{
		TextureInfo textureInfo;
		std::vector<uint8_t> data;

		if (!Utils::LoadTextureDataFromFile(path, textureInfo, data))
			return nullptr;

		Ref<Texture2D> texture = Create(textureInfo, std::move(data));
		if (texture) {
			texture->m_Path = path.string();
		}
		return texture;
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
		std::string path = texture->GetPath();
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
		if(texture)
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
		case RendererAPI::API::Vulkan:
			return MakeRef<TextureCube>(textures);

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
		case RendererAPI::API::Vulkan:
			return MakeRef<TextureCube>(path);

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
		case RendererAPI::API::Vulkan:
			return MakeRef<TextureCube>(format, width, height);

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
		case RendererAPI::API::Vulkan:
			return MakeRef<TextureCube>();

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

