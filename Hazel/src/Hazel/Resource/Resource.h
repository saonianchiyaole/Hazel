#pragma once

#include "Hazel/Core/UUID.h"
#include <memory>
#include <atomic>
#include <functional>

namespace Hazel {	

	class Shader;
	class Texture;
	

	template<typename T>
	class ResourceManager;

	template<typename HandleT, typename ProxyT>
	class DeviceResourceTable;

	template<typename T>
	struct ResourceHandle{

	public:
		
		ResourceHandle() {
			
		}		

		const uint32_t GetIndex() const {

			return index;
		}

		const uint32_t GetGeneration() const {
			return generation;
		}
		
		
		bool operator==(const ResourceHandle<T>& other) const {
			return (index == other.index) && (generation == other.generation);
		}
					
		bool operator!=(const ResourceHandle<T>& other) const {
			return !((*this) == other);
		}

		bool IsValid() {
			return generation == 0;
		}

	private:
		uint32_t index = 0;
		uint32_t generation = 0;		

		
		template<typename U>
		friend class ResourceManager;

		template<typename HandleT, typename ProxyT>
		friend class DeviceResourceTable;
	};

	
	template<typename T>
	using Handle = ResourceHandle<T>;


	using ShaderHandle = ResourceHandle<Shader>;
	using TextureHandle = ResourceHandle<Texture>;

	enum ResourceFlag {
		Valid = 1,
		Loading = 2,
		Invalid = 0
	};

	enum ResourceSource {
		File,
		Program
	};

	enum class ResourceType {
		None,
		Mesh,
		Material,
		Shader
	};

	class Resource{
	public:

		Resource() = default;


		ResourceFlag		GetFlag		()					{ return m_Flag; }
		void				SetFlag		(ResourceFlag flag)	{ m_Flag = flag; }

		virtual ResourceType GetResourceType(){
			return ResourceType::None;
		}


		bool IsValid() { return m_Flag == ResourceFlag::Valid; }		

	
	protected:			
		
		std::atomic<ResourceFlag> m_Flag = ResourceFlag::Invalid;
		std::atomic<uint32_t> m_Version = 0;

		std::mutex m_Mutex;


	};
}

namespace std {
	template<typename T>
	struct hash<Hazel::ResourceHandle<T>> {
		size_t operator()(const Hazel::ResourceHandle<T>& handle) const noexcept {
			const size_t hashIndex = hash<uint32_t>{}(handle.GetIndex());
			const size_t hashGen = hash<uint32_t>{}(handle.GetGeneration());
			return hashIndex ^ (hashGen + 0x9e3779b9 + (hashIndex << 6) + (hashIndex >> 2));
		}
	};
}
