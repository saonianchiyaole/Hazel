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
			return this->size;
		}
		void Copy(void* srcData, uint64_t size) {
			memcpy(srcData, data, size);
		}


		void Allocate(uint64_t sizeVal) {

			if (size != 0) {
				Free();
			}
			size = sizeVal;
			if (size != 0)
				data = new char[size];
			
		}

		void Free() {
			if (size != 0)
				delete[] data;
			size = 0;
		}

		template<typename T>
		T* Read(uint64_t offset = 0) {
			return (T*)((char*)data + offset);
		}

		void Write(const void* data, uint64_t size, uint64_t offset = 0) {
			HZ_CORE_ASSERT(offset + size <= this->size, "Buffer overflow!");
			memcpy((char*)this->data + offset, data, size);
		}

		operator bool() {
			return data != nullptr;
		}

		void ZeroInitialize()
		{
			if (data)
				memset(data, 0, size);
		}
	private:

		void* data = nullptr;
		uint64_t size = 0;
	};
}