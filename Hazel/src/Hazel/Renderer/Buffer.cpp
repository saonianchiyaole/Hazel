#include "hzpch.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Hazel/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Hazel {
		
	namespace Utils {


		uint32_t ShaderDataTypeSize(ShaderDataType type) {
			switch (type) {
			case ShaderDataType::Float:			return 4;
			case ShaderDataType::Float2:		return 4 * 2;
			case ShaderDataType::Float3:		return 4 * 3;
			case ShaderDataType::Float4:		return 4 * 4;
			case ShaderDataType::Vec2:			return 4 * 2;
			case ShaderDataType::Vec3:			return 4 * 3;
			case ShaderDataType::Vec4:			return 4 * 4;
			case ShaderDataType::Mat3:			return 4 * 3 * 3;
			case ShaderDataType::Mat4:			return 4 * 4 * 4;
			case ShaderDataType::Int:			return 4;
			case ShaderDataType::Int2:			return 4 * 2;
			case ShaderDataType::Int3:			return 4 * 3;
			case ShaderDataType::Int4:			return 4 * 4;
			case ShaderDataType::Bool:			return 1;
			case ShaderDataType::Sampler2D: {
				switch (RendererAPI::GetAPI()) {
				case RendererAPI::API::None:
					return 0;
				case RendererAPI::API::OpenGL:
					return sizeof(OpenGLTexture2D);
				}
			}
			}

			HZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}

	}


	Ref<VertexBuffer> VertexBuffer::Create(size_t size, std::vector<uint8_t> data) {

		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLVertexBuffer>(size, data);
		case RendererAPI::API::Vulkan:
			return MakeRef<VertexBuffer>(size);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		std::vector<uint8_t> data;
		if (vertices && size > 0) {
			const uint8_t* begin = reinterpret_cast<const uint8_t*>(vertices);
			data.assign(begin, begin + size);
		}

		return Create(static_cast<size_t>(size), std::move(data));
	}

	Ref<VertexBuffer> VertexBuffer::Create(void* vertices, uint32_t size)
	{
		std::vector<uint8_t> data;
		if (vertices && size > 0) {
			const uint8_t* begin = static_cast<const uint8_t*>(vertices);
			data.assign(begin, begin + size);
		}

		return Create(static_cast<size_t>(size), std::move(data));
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		return Create(static_cast<size_t>(size));
	}

	VertexBuffer::~VertexBuffer() {

	}

	Ref<IndexBuffer> IndexBuffer::Create(size_t size, std::vector<uint8_t> data) {

		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLIndexBuffer>(size, data);
		case RendererAPI::API::Vulkan:
			return MakeRef<IndexBuffer>(size);
		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		const size_t size = static_cast<size_t>(count) * sizeof(uint32_t);
		std::vector<uint8_t> data;
		if (indices && count > 0) {
			const uint8_t* begin = reinterpret_cast<const uint8_t*>(indices);
			data.assign(begin, begin + size);
		}

		return Create(size, std::move(data));
	}

	Ref<IndexBuffer> IndexBuffer::Create(void* indices, uint32_t count)
	{
		const size_t size = static_cast<size_t>(count) * sizeof(uint32_t);
		std::vector<uint8_t> data;
		if (indices && count > 0) {
			const uint8_t* begin = static_cast<const uint8_t*>(indices);
			data.assign(begin, begin + size);
		}

		return Create(size, std::move(data));
	}

	IndexBuffer::~IndexBuffer() {

	}

	// param binding is useless for vulkan api for now
	Ref<UniformBuffer> UniformBuffer::Create(size_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported")
		case RendererAPI::API::OpenGL:
			return MakeRef<OpenGLUniformBuffer>(size, binding);
		case RendererAPI::API::Vulkan:
			return MakeRef<UniformBuffer>(size, binding);

		}
		HZ_CORE_ASSERT(false, "Can't recognize the API!")
			return nullptr;
	}


	// ----------------------------------------------------- Uniform Buffer Set ----------------------------------------




	Ref<UniformBufferSet> UniformBufferSet::Create(uint32_t amount, size_t size) {


		Ref<UniformBufferSet> uniformBufferSet = MakeRef<UniformBufferSet>(amount, size);
		

		return uniformBufferSet;
		

	}

	Ref<UniformBufferSet> UniformBufferSet::Create(size_t size) {

		uint32_t frameInFlight = Renderer::GetFrameInFlight();
		return UniformBufferSet::Create(frameInFlight, size);

	}

	UniformBufferSet::UniformBufferSet(uint32_t amount, size_t size) {


		m_UniformBuffers.resize(amount);
		for (uint32_t i = 0; i < amount; i++) {
			m_UniformBuffers[i] = UniformBuffer::Create(size, 0);
		}

	}


	Ref<UniformBuffer> UniformBufferSet::Get() {
		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();
		return Get(frameIndex);
	}

	Ref<UniformBuffer> UniformBufferSet::Get(uint32_t frameIndex) {

		return m_UniformBuffers[frameIndex];
	}



}


