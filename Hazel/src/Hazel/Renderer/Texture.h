#pragma once


namespace Hazel {

	enum class TextureType {
		None, Albedo, Roughness, Metalness, Normal
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
	public:
		Texture2D() = default;
		~Texture2D() = default;
		//static Ref<Texture2D> Create(const std::string& path);

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		std::string GetPath() const;

		static Ref<Texture2D> Create(const uint32_t width, const uint32_t height);
		static Ref<Texture2D> Create(std::filesystem::path path);
		virtual void SetData(const void* data, const uint32_t size) = 0;
		virtual void SetType(TextureType type);

		TextureType GetType();

		virtual const uint32_t GetRendererID() = 0;
		bool IsLoaded() { return m_Loaded; }

	protected:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		TextureType m_Type;
		bool m_Loaded = false;
	};

}