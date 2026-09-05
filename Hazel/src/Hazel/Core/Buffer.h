#pragma once
#include "Hazel/Core/Log.h"

namespace Hazel
{
	class Buffer {
	public:
		friend class Material;

		Buffer() = default;

		Buffer(uint64_t size) {
			Allocate(size);
			ZeroInitialize();
		}

		Buffer(const Buffer& other) {
			Allocate(other.m_Size);
			CopyFrom(other.m_Data, m_Size);
		}

		Buffer(Buffer&& other) noexcept {
			this->m_Data = other.m_Data;
			this->m_Size = other.m_Size;
			other.m_Data = nullptr;
			other.m_Size = 0;
		}

		~Buffer() {
			Free();
		}


		inline uint64_t GetSize() const {
			return this->m_Size;
		}

		inline void CopyFrom(void* srcData, uint64_t size) {

			HZ_CORE_ASSERT(m_Size >= size, "Data out of range! my Size: {} byte, copy Size: {} byte\n", m_Size, size);

			memcpy(m_Data, srcData, size);
		}


		inline void Allocate(uint64_t size) {

			if (m_Size == size) {
				return;
			}

			Free();

			m_Size = size;
		

			m_Data = m_Size ? (void*)malloc(m_Size) : nullptr;
		}		

		inline void Free() {
			if (m_Size != 0 && m_Data)
				free(m_Data);
			m_Data = nullptr;
			m_Size = 0;
		}

		template<typename T>
		inline T* Read(uint64_t offset = 0) {
			return (T*)((char*)m_Data + offset);
		}

		inline void Write(const void* data, uint64_t size, uint64_t offset = 0) {
			HZ_CORE_ASSERT(offset + size <= m_Size, "Buffer overflow!");
			memcpy((char*)this->m_Data + offset, data, size);
		}

		// directly copy the input pointer's value to buffer's, don't suggest to use this
		inline void Write(void* ptr) {
			HZ_CORE_ASSERT(m_Size >= 8, "Buffer overflow");
			m_Data = ptr;
		}

		operator bool() {
			return m_Data != nullptr;
		}

		operator void*() {
			return m_Data;
		}

		void ZeroInitialize()
		{
			if (m_Data)
				memset(m_Data, 0, m_Size);
		}
	private:

		void* m_Data = nullptr;
		uint64_t m_Size = 0;
	};
}
