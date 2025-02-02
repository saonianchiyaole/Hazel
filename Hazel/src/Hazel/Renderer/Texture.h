#pragma once


namespace Hazel {

	enum class TextureType {
		None, Albedo, Roughness, Metalness, Normal, Other
	};

	enum class TextureFormat {
		None,
		RG,
		RGB,
		RGBA,
		Float16
	};


	class Texture {
	public:
		Texture() = default;
		~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator ==(Texture& other) const = 0;
	};

	class Texture2D : public Texture {

	friend class OpenGLTextureCube;
	friend class TextureCube;

	public:
		Texture2D() = default;
		~Texture2D() = default;
		//static Ref<Texture2D> Create(const std::string& path);

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		std::string GetPath() const;

		static Ref<Texture2D> Create(const uint32_t width, const uint32_t height);
		static Ref<Texture2D> Create(std::filesystem::path path);
		// Pre create to allocate memory
		static Ref<Texture2D> PreCreate();
		static Texture2D* PreCreateNakedPointer();

		virtual void SetData(const void* data, const uint32_t size) = 0;
		virtual void SetType(TextureType type);

		unsigned int GetDataFormat() { return m_DataFormat; }
		unsigned int GetInternalFormat() { return m_InternalFormat; }
		unsigned int GetDataType() { return m_DataType; }

		TextureType GetType();
		uint32_t GetSlot();
		void SetSlot(uint32_t slot);

		virtual const uint32_t GetRendererID() = 0;
		bool IsLoaded() { return m_Loaded; }

		bool operator == (Texture2D& other) {
			return this->m_Path == other.m_Path;
		}

	protected:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		TextureType m_Type = TextureType::None;
		uint32_t m_Slot = 0;
		//Format
		TextureFormat m_TextureFormat;
		unsigned int m_InternalFormat = 0;
		unsigned int m_DataFormat = 0;
		unsigned int m_DataType = 0;
		
		bool m_Loaded = false;
		bool m_IsHDR = false;
	};


	class TextureCube{
	public:
		static Ref<TextureCube> Create(std::vector<Ref<Texture2D>> textures);
		static Ref<TextureCube> Create(const std::string& path);
		static Ref<TextureCube> Create();
		virtual uint32_t GetRendererID() = 0;
		std::vector<Ref<Texture2D>> GetTexutres();
		virtual void SetTexture(Ref<Texture2D> texture, uint32_t slot) = 0;
		bool IsLoaded();
	protected:
		std::vector<Ref<Texture2D>> m_Textures;
		bool m_IsLoaded = false;
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