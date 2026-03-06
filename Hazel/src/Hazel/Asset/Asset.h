#pragma once

#include "Hazel/Core/UUID.h"
#include <memory>

namespace Hazel {	


	struct AssetHandle{

		uint32_t index;
		uint32_t generation = 0;


		bool operator==(const AssetHandle& other) const {
			return (index == other.index) && (generation == other.generation);
		}
					
		bool operator!=(const AssetHandle& other) const {
			return !((*this) == other);
		}


	};

	
	

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
		Texture2D,
		Texture3D,
		TextireCube,
		Mesh,
		Material,
		Shader
	};

	class Asset : public std::enable_shared_from_this<Asset>{
	public:

		Asset();

		AssetHandle GetHandle	()					{ return m_Handle; }
		AssetFlag	GetFlag		()					{ return m_Flag; }
		void		SetFlag		(AssetFlag flag)	{ m_Flag = flag; }

		virtual AssetType GetAssetType(){
			return AssetType::None;
		}


		bool IsValid() { return m_Flag == AssetFlag::Valid; }

		bool operator==(const Asset& other) const{			
			return this->m_Handle == other.m_Handle;
		}

		bool operator!=(const Asset& other) const {
			return this->m_Handle != other.m_Handle;
		}

		template<typename T>
		Ref<T> As(){

			return std::dynamic_pointer_cast<T>(shared_from_this());
		}

	protected:
		friend class AssetManager;
		AssetHandle m_Handle;
		
		std::atomic<AssetFlag> m_Flag = AssetFlag::Invalid;
		std::atomic<uint32_t> m_Generation = 0;

		std::mutex m_Mutex;


	};
}

namespace std {
	template<>
	struct hash<Hazel::AssetHandle> {
		size_t operator()(const Hazel::AssetHandle& handle) const {
			auto hashIndex = hash<uint32_t>{}(handle.index);
			auto hashGen = hash<uint32_t>{}(handle.generation);

			return hashIndex ^ (hashGen + 0x9e3779b9 + (hashIndex << 6) + (hashIndex >> 2));
		}
	};
}