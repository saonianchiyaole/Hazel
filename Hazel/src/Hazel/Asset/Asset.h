#pragma once

#include "Hazel/Core/UUID.h"
#include <memory>

namespace Hazel {
	using AssetHandle = UUID;

	enum AssetFlag {
		Valid = 1,
		Loading = 2,
		Invalid = 0
	};

	enum AssetSource {
		File,
		Program
	};

	enum class AssetType {
		None,
		Texture,
		Mesh,
		Material,
		Shader
	};

	class Asset : public std::enable_shared_from_this<Asset>{
	public:

		Asset();

		AssetHandle GetHandle() { return m_Handle; }
		AssetFlag GetFlag() { return m_Flag; }

		virtual AssetType GetAssetType(){
			return AssetType::None;
		}

		bool operator == (const Asset& other) const{
			return this->m_Handle == other.m_Handle;
		}

		bool operator != (const Asset& other) const {
			return !(this->m_Handle == other.m_Handle);
		}

		template<typename T>
		Ref<T> As(){

			return std::dynamic_pointer_cast<T>(shared_from_this());
		}

	protected:
		AssetHandle m_Handle = UUID();
		AssetFlag m_Flag = AssetFlag::Invalid;
		std::mutex m_Mutex;


	};
}