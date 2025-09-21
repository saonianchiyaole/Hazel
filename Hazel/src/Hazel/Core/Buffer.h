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

		~Buffer() {
			Free();
		}


		uint64_t GetSize() {
			return this->m_Size;
		}

		void CopyFrom(void* srcData, uint64_t size) {
			memcpy(m_Data, srcData, size);
		}


		void Allocate(uint64_t size) {

			if (m_Size != 0) {
				Free();
			}
			m_Size = size;
			if (m_Size != 0)
				m_Data = (void*)malloc(m_Size);
			
		}

		void Free() {
			if (m_Size != 0 && m_Data)
				free(m_Data);
			m_Size = 0;
		}

		template<typename T>
		T* Read(uint64_t offset = 0) {
			return (T*)((char*)m_Data + offset);
		}

		void Write(const void* data, uint64_t size, uint64_t offset = 0) {
			HZ_CORE_ASSERT(offset + size <= m_Size, "Buffer overflow!");
			memcpy((char*)this->m_Data + offset, data, m_Size);
		}

		// directly copy the input pointer's value to buffer's, don't suggest to use this
		void Write(void* ptr) {
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