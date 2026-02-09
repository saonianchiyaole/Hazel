#include "hzpch.h"
#include "AssetManager.h"

namespace Hazel {


	std::vector<Ref<Asset>> AssetManager::s_Assets;

	std::stack<AssetHandle> AssetManager::s_FreeIndexStack;

	std::mutex AssetManager::s_AssetLock;

}


