#pragma once


namespace Hazel {



	class GPUBuffer{

	public:

		GPUBuffer() = default;
		GPUBuffer(size_t size) : m_Size(size){}

	protected:

		uint64_t m_Size = 0;

	};


	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, 
		Vec2, Vec3, Vec4,
		Mat3, Mat4,
		Int, Int2, Int3, Int4,
		Bool,
		Sampler2D, SamplerCube,		
	};


	namespace Utils {
		uint32_t ShaderDataTypeSize(ShaderDataType type);
	}


	struct BufferElement {
		std::string Name;
		uint32_t Offset;
		uint32_t Size;
		ShaderDataType Type;
		bool Normalized;

		BufferElement() {}
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			:Name(name), Type(type), Size(Utils::ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {}




		uint32_t GetComponentCount() const {
			switch (Type) {
			case ShaderDataType::Float:			return 1;
			case ShaderDataType::Float2:		return 2;
			case ShaderDataType::Float3:		return 3;
			case ShaderDataType::Float4:		return 4;
			case ShaderDataType::Int:			return 1;
			case ShaderDataType::Int2:			return 2;
			case ShaderDataType::Int3:			return 3;
			case ShaderDataType::Int4:			return 4;
			case ShaderDataType::Mat3:			return 9;
			case ShaderDataType::Mat4:			return 16;
			case ShaderDataType::Bool:			return 1;
			case ShaderDataType::Sampler2D:		return 1;
			case ShaderDataType::SamplerCube:	return 1;
			}

			HZ_CORE_ASSERT(false, "Unknown ElementType!");
			return 0;
		}
	};


	class BufferLayout {
	public:
		BufferLayout() {}
		BufferLayout(std::vector<BufferElement> elements) {
			m_Elements = elements;
			CalculateOffsetAndStride();
		}

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		inline uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end()   { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetAndStride() {
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements) {
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer : public GPUBuffer{
	public:
		VertexBuffer() = default;
		VertexBuffer(size_t size) : GPUBuffer(size){}
		virtual ~VertexBuffer();


		virtual void Bind() const {}
		virtual void Unbind() const {}


		virtual void SetData(const void* data, const uint32_t size) {}

		virtual void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
		virtual const BufferLayout& GetLayout() const { return m_Layout; }

		static Ref<VertexBuffer> Create(size_t size, std::vector<uint8_t> data = std::vector<uint8_t>{});
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
		static Ref<VertexBuffer> Create(void* vertices, uint32_t size);
		static Ref<VertexBuffer> Create(uint32_t size);

	protected:
	 
		BufferLayout m_Layout;
	};

	class IndexBuffer : public GPUBuffer {
	public:
		IndexBuffer() = default;
		IndexBuffer(size_t size) : GPUBuffer(size), m_Count(static_cast<uint32_t>(size / sizeof(uint32_t))) {}

		virtual ~IndexBuffer();

		virtual void Bind() const {}
		virtual void Unbind() const {}

		virtual uint32_t GetCount() const { return m_Count; }

		static Ref<IndexBuffer> Create(size_t size, std::vector<uint8_t> data = std::vector<uint8_t>{});
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
		static Ref<IndexBuffer> Create(void* indices, uint32_t count);

	protected:
		uint32_t m_Count = 0;

	};

	class UniformBuffer : public GPUBuffer {
	public:
		UniformBuffer() = default;
		UniformBuffer(size_t size, uint32_t binding) : GPUBuffer(size), m_Binding(binding) {}
		virtual ~UniformBuffer() = default;

		static Ref<UniformBuffer> Create(size_t size, uint32_t binding);

		virtual void SetData(const void* data, size_t size, size_t offset = 0) {}

	protected:
		uint32_t m_Binding = 0;
	
	};

	// used for per frame 
	class UniformBufferSet {
	public:
		

		static Ref<UniformBufferSet> Create(uint32_t amount, size_t size);
		static Ref<UniformBufferSet> Create(size_t size);

		Ref<UniformBuffer> Get();

		Ref<UniformBuffer> Get(uint32_t frameIndex);

		inline uint32_t GetUniformBufferAmount() {
			return static_cast<uint32_t>(m_UniformBuffers.size());
		}

	//private:


		UniformBufferSet(uint32_t amount, size_t size);

		std::vector<Ref<UniformBuffer>> m_UniformBuffers;

	};

}
