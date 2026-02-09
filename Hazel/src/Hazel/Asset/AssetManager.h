#pragma once
#include "Hazel/Asset/Asset.h"
#include "Hazel/Core/Core.h"
#include <unordered_map>
#include <memory>
#include <atomic>
#include <stack>

namespace Hazel {

	namespace Utils {
		//static bool ChangeToCorrectAssetType(Ref<Asset> asset);
	}

	class AssetManager {

	public:
		//static void GetAssetManager();
		
		static Ref<Asset> GetAsset(AssetHandle& handle) {
			HZ_CORE_ASSERT(HasAsset(handle),
							"Invalid handle index: {}, generation: {}!", handle.index, handle.generation)
			return s_Assets[handle.index];
		}
		static bool HasAsset(AssetHandle& handle) {
			return !(handle.index >= s_Assets.size() || handle.generation != s_Assets[handle.index]->m_Handle.generation);
		}		

		static AssetHandle AddAsset(Ref<Asset> asset) {
			
			{
				std::unique_lock<std::mutex> lock(s_AssetLock);

				AssetHandle handle;

				if (!s_FreeIndexStack.empty()) {					
					handle = s_FreeIndexStack.top();
					s_FreeIndexStack.pop();
					handle.generation++;
				}
				else {					
					handle.index = s_Assets.size();
					s_Assets.push_back(nullptr);
				}

				asset->m_Handle = handle;
										
				s_Assets[asset->GetHandle().index] = asset;
			}			

			return asset->GetHandle();
		}

		template<typename T, typename ... Args>
		static Ref<T> Create(Args&& ... args){

			bool isBaseOfAsset = std::is_base_of<Asset, T>::value;
			if (!isBaseOfAsset)
				return nullptr;
			Ref<T> asset = MakeRef<T>(std::forward<Args>(args)...);
			AddAsset(asset);
			return asset;
		}

	private:
		//static Ref<AssetManager> s_AssetManager;
		static std::vector<Ref<Asset>> s_Assets;
		static std::stack<AssetHandle> s_FreeIndexStack;
		
		static std::mutex s_AssetLock;		
	};

}