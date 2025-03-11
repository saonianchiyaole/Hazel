#pragma once
#include "Hazel/Asset/Asset.h"
#include "Hazel/Core/Core.h"
#include "unordered_map"
#include <memory>

namespace Hazel {

	namespace Utils {
		//static bool ChangeToCorrectAssetType(Ref<Asset> asset);
	}

	class AssetManager {

	public:
		//static void GetAssetManager();

		static Ref<Asset> GetAsset(AssetHandle& handle) {
			return s_Assets[handle];
		}
		static bool HasAsset(AssetHandle& handle) {
			return s_Assets[handle] != nullptr;
		}

		static bool HasValidAsset(AssetHandle& handle) {
			return s_Assets[handle] != nullptr && s_Assets[handle]->GetFlag();
		}

		static void AddAsset(Ref<Asset> asset) {
			s_Assets[asset->GetHandle()] = asset;
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
		static std::unordered_map<uint64_t, Ref<Asset>> s_Assets;

	};

}