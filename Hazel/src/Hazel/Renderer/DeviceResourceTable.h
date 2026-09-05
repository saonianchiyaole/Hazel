#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <algorithm>

namespace Hazel {

	template<typename HandleT, typename ProxyT>
	class DeviceResourceTable {
	public:
		void Add(HandleT handle, ProxyT proxy) {
			if (handle.index >= m_Slots.size()) {
				m_Slots.resize(handle.index + 1);
			}

			Slot& slot = m_Slots[handle.index];
			slot.proxy = std::move(proxy);
			slot.alive = true;
			slot.generation = handle.generation;

			auto freeIt = std::find(m_FreeList.begin(), m_FreeList.end(), handle.index);
			if (freeIt != m_FreeList.end()) {
				m_FreeList.erase(freeIt);
			}
		}

		ProxyT* Get(HandleT handle) {
			Slot* slot = GetSlot(handle);
			return slot ? &slot->proxy : nullptr;
		}

		const ProxyT* Get(HandleT handle) const {
			const Slot* slot = GetSlot(handle);
			return slot ? &slot->proxy : nullptr;
		}

		bool Has(HandleT handle) const {
			return GetSlot(handle) != nullptr;
		}

		void Remove(HandleT handle) {
			Slot* slot = GetSlot(handle);
			if (!slot)
				return;

			slot->proxy = ProxyT{};
			slot->alive = false;
			m_FreeList.push_back(handle.index);
		}

		void Clear() {
			m_Slots.clear();
			m_FreeList.clear();
		}

	private:
		struct Slot {
			ProxyT proxy{};
			uint32_t generation = 0;
			bool alive = false;
		};

		Slot* GetSlot(HandleT handle) {
			if (handle.index >= m_Slots.size())
				return nullptr;

			Slot& slot = m_Slots[handle.index];
			if (!slot.alive || slot.generation != handle.generation)
				return nullptr;

			return &slot;
		}

		const Slot* GetSlot(HandleT handle) const {
			if (handle.index >= m_Slots.size())
				return nullptr;

			const Slot& slot = m_Slots[handle.index];
			if (!slot.alive || slot.generation != handle.generation)
				return nullptr;

			return &slot;
		}

	private:
		std::vector<Slot> m_Slots;
		std::vector<uint32_t> m_FreeList;
	};

}
