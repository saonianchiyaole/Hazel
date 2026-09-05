#pragma once


#include "Hazel/Resource/Resource.h"

namespace Hazel {

	

	namespace Utils {
		uint32_t CalculateMipmapCount(uint32_t width, uint32_t height);

		template<typename T>
		uint32_t GetSizeOfTextureType();		

	}


	enum class TextureType {
		None, Albedo, Roughness, Metalness, Normal, Other
	};

	enum class PixelFormat {
		None,
		R,   //R8
		RG,  //R8G8
		RGB, //R8G8B8
		RGBA, //R8G8B8A8
		Float16,
		DEPTH24STENCIL8,
		// Defaults
		Depth = DEPTH24STENCIL8,
	};

	using TextureFormat = PixelFormat;

	enum class TextureUsage {
		None,
		Attachment,
		Texture
	};


	struct TextureInfo {

		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 0;
		PixelFormat format = PixelFormat::RGBA;
		TextureUsage usage = TextureUsage::Texture;
		bool isHDR = false;		
	};

	namespace Utils {
		uint32_t GetPixelFormatChannelCount(PixelFormat format);
		uint32_t GetPixelFormatSourceBytesPerChannel(PixelFormat format);
		bool LoadTextureDataFromFile(const std::filesystem::path& path, TextureInfo& textureInfo, std::vector<uint8_t>& data);
	}


	class Texture {
	public:
		Texture() = default;
		~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;		

		virtual bool operator ==(Texture& other) const = 0;
	};

	class Texture2D : public Texture {

	friend class OpenGLTextureCube;
	friend class TextureCube;
	friend class OpenGLEnvironment;

	public:
		Texture2D() = default;
		~Texture2D() = default;
		//static Ref<Texture2D> Create(const std::string& path);

		static Ref<Texture2D> Create(TextureInfo textureInfo, std::vector<uint8_t> data = {});
		static Ref<Texture2D> Create(std::filesystem::path path);
		// Pre create to allocate memory
		
		//Get
		virtual uint32_t GetWidth() const override;
		virtual uint32_t GetHeight() const override;
		std::string GetPath() const;

		TextureType GetType();
		PixelFormat GetTextureFormat() { return m_TextureFormat; }
		unsigned int GetDataFormat() { return m_DataFormat; }
		unsigned int GetInternalFormat() { return m_InternalFormat; }
		unsigned int GetDataType() { return m_DataType; }

		uint32_t GetSlot();

		//Set
		virtual void SetData(const void* data, const uint32_t size) = 0;
		virtual void SetType(TextureType type);
		void SetSlot(uint32_t slot);

		virtual const uint32_t GetRendererID() = 0;
		
		virtual bool operator == (Texture2D& other) const {
			return this->m_Path == other.m_Path;
		}

	protected:
		
		TextureInfo m_Info;

		std::string m_Path;
		uint32_t m_Width, m_Height;
		TextureType m_Type = TextureType::None;
		uint32_t m_Slot = 0;
		//Format
		PixelFormat m_TextureFormat;
		TextureUsage m_Usage;
		unsigned int m_InternalFormat = 0;
		unsigned int m_DataFormat = 0;
		unsigned int m_DataType = 0;
						
		bool m_IsHDR = false;
	};


	class TextureCube {
	public:
		friend class Environment;
		friend class OpenGLEnvironment;

		static Ref<TextureCube> Create(std::vector<Ref<Texture2D>> textures);
		static Ref<TextureCube> Create(const std::string& path);
		static Ref<TextureCube> Create(PixelFormat format, const uint32_t width, const uint32_t height);
		static Ref<TextureCube> Create();

		//Get 
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		std::string GetPath() const;
		uint32_t GetMipmapCount();

		virtual void Bind(uint32_t slot = 0) = 0;
		virtual uint32_t GetRendererID() = 0;
		std::vector<Ref<Texture2D>> GetTexutres();
		virtual void SetTexture(Ref<Texture2D> texture, uint32_t slot) = 0;
		bool IsLoaded();
		bool IsHDR();
	protected:


		void SetIsLoaded(bool value) { m_IsLoaded = value; }

		std::vector<Ref<Texture2D>> m_Textures;

		PixelFormat m_TextureFormat;
		unsigned int m_InternalFormat = 0;
		unsigned int m_DataFormat = 0;
		unsigned int m_DataType = 0;
		std::string m_Path;
		uint32_t m_Width, m_Height;

		bool m_IsLoaded = false;
		bool m_IsHDR = false;
	};



	class TextureLibrary {
	public:
		static void Add(const Ref<Texture2D> texture);
		static Ref<Texture2D> Load(const std::string& path);
		
		static bool Exists(const std::string& name);
		static Ref<Texture2D> Get(const std::string& name);
		
		
	private:

		friend class Editor;
		static std::unordered_map<std::string, Ref<Texture2D>> m_Textures;
	};

}
