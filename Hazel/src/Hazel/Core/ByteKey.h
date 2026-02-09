#pragma once

namespace Hazel {


	// FNV-1a 32位哈希算法固定参数
	constexpr uint32_t FNV1A_32_OFFSET = 0x811C9DC5u;
	constexpr uint32_t FNV1A_32_PRIME = 0x01000193u;


	class ByteKey {
	public:

		ByteKey() = default;

		ByteKey(ByteKey&& other) {
			this->m_Bytes = std::move(other.m_Bytes);
		}

		template <typename T>
		void AddBytes(const T& value) {
			const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
			m_Bytes.insert(m_Bytes.end(), bytes, bytes + sizeof(T));
		}

		uint64_t Hash() const {

			uint32_t hash = FNV1A_32_OFFSET;
			
			for (uint8_t byte : m_Bytes) {
				hash ^= static_cast<uint32_t>(byte);   
				hash *= FNV1A_32_PRIME;                
			}
			return hash;

		}


	private:

		std::vector<uint8_t> m_Bytes;

	};

	
	template<typename T>
	using ByteKeyMap = std::unordered_map<ByteKey, T>;


}

namespace std {

	template<>
	struct hash<Hazel::ByteKey> {

		std::size_t operator()(const Hazel::ByteKey& key) const {
			return hash<uint64_t>()(key.Hash());
		}
	};

}