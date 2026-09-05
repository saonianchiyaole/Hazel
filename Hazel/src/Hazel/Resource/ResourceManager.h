#pragma once
#include "Hazel/Resource/Resource.h"
#include "Hazel/Core/Core.h"
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <stack>
#include <vector>

namespace Hazel {

	namespace Utils {
		//static bool ChangeToCorrectAssetType(Ref<GpuResource> resource);
	}

		
	template<typename T>
	class ResourceManager {
	public:
		//static void GetAssetManager();
				
		static Ref<T> Get(const ResourceHandle<T>& handle) {
			HZ_CORE_ASSERT(Has(handle),
							"Invalid handle index: {}, generation: {}!", handle.GetIndex(), handle.GetGeneration())
			return s_Resources[handle.GetIndex()];
		}

		static bool Has(const ResourceHandle<T>& handle) {
			if (handle.GetIndex() >= s_Resources.size()) {
				return false;
			}
			if (handle.GetGeneration() != s_ResourceHandles[handle.GetIndex()].GetGeneration()) {
				return false;
			}
			return s_Resources[handle.GetIndex()] != nullptr;
		}		
		

		template<typename ... Args>
		static ResourceHandle<T> Create(Args&& ... args) {

			Ref<T> resource = CreateResource(std::forward<Args>(args)...);
			auto handle = Add(resource);
			return handle;

		}

		static ResourceHandle<T> Add(const Ref<T> resource) {

			auto handle = AllocateHandle();
			s_Resources[handle.index] = resource;
			return handle;

		}

		static void SetResource(const ResourceHandle<T>& handle, const Ref<T>& resource) {
			std::unique_lock<std::mutex> lock(s_AssetLock);
			HZ_CORE_ASSERT(handle.GetIndex() < s_Resources.size(), "Invalid resource handle index: {}", handle.GetIndex());
			HZ_CORE_ASSERT(handle.GetGeneration() == s_ResourceHandles[handle.GetIndex()].GetGeneration(), "Invalid resource handle generation: {}", handle.GetGeneration());

			s_Resources[handle.GetIndex()] = resource;
		}


	private:

		template<typename ...Args>
		static Ref<T> CreateResource(Args&&... args) {
			
			return MakeRef<T>(std::forward<Args>(args)...);

		}

		
		static ResourceHandle<T> AllocateHandle() {
			std::unique_lock<std::mutex> lock(s_AssetLock);

			ResourceHandle<T> handle;

			if (!s_FreeIndexStack.empty()) {
				handle = s_FreeIndexStack.top();
				s_FreeIndexStack.pop();
				handle.generation++;
			}
			else {
				handle.index = static_cast<uint32_t>(s_Resources.size());
				handle.generation = 1;
				s_Resources.push_back(nullptr);
				s_ResourceHandles.push_back(ResourceHandle<T>());
			}

			s_Resources[handle.GetIndex()] = nullptr;
			s_ResourceHandles[handle.GetIndex()] = handle;
			return handle;
		}
										

	private:
		// strong ref count manage by shared_ptr
		inline static std::vector<Ref<T>> s_Resources;
		// weak ref count, because shared_ptr will mange the life time automatically, we only need to check the weak ref count;
		inline static std::vector<ResourceHandle<T>> s_ResourceHandles;
		inline static std::stack<ResourceHandle<T>> s_FreeIndexStack;
		inline static std::mutex s_AssetLock;		
	};

}
