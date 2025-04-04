#pragma once


namespace Hazel {

	class UUID {
	public:

		UUID();
		UUID(uint64_t id);

		operator uint64_t() const {
			return m_UUID;
		}

		operator uint64_t() {
			return m_UUID;
		}

		bool operator == (const UUID& other) {
			return this->m_UUID == other.m_UUID;
		}

	private:
		uint64_t m_UUID;
	};

}

namespace std {

	template<>
	struct hash<Hazel::UUID> {

		std::size_t operator()(const Hazel::UUID& uuid) const {
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}