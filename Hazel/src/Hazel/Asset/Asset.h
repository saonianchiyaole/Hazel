#pragma once

#include "Hazel/Core/UUID.h"

namespace Hazel {
	using AssetHandle = UUID;

	enum AssetFlag {
		Valid = 1,
		Invalid = 0
	};

	enum class AssetType {
		None,
		Texture,
		Mesh,
		Material,
		Shader
	};

	class Asset {
	public:
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
		T As(){
			return Ref<T>(this);
		}

	protected:
		AssetHandle m_Handle = 0;
		AssetFlag m_Flag = AssetFlag::Invalid;


	};
}