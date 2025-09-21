#pragma once

#include <memory>
#include <vector>

#ifdef HZ_PLATFORM_WINDOWS
#if HZ_DYNAMIC_LINK
	#ifdef HZ_BUILD_DLL
		#define HAZEL_API __declspec(dllexport)
	#else 
		#define HAZEL_API __declspec(dllimport)
	#endif // DEBUG
#else 
	#define HAZEL_API
#endif
#else 
	#error Hazel only support windows!
#endif


#ifdef HZ_ENABLE_ASSERTS
	#define HZ_ASSERT(x, ...) { if(!(x)) { HZ_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define HZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_ERROR("{} : Line: {} :Assertion Failed: {0}", __FILE__, __LINE__, __VA_ARGS__); __debugbreak(); } }
#else
	#define HZ_ASSERT(x, ...)
	#define HZ_CORE_ASSERT(x, ...)
#endif // HZ_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define HZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)


namespace Hazel {


	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using WeakRef = std::weak_ptr<T>;
	
	
	template<typename T, typename ... Args>
	constexpr Scope<T> MakeScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
	template<typename T, typename ... Args>
	constexpr Ref<T> MakeRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
	

	// default use static_pointer_cast
	template<typename DstType, typename SrcType>
	constexpr std::vector<Ref<DstType>> RefVectorStaticCast(std::vector<SrcType> array) {

		std::vector<Ref<DstType>> dstTypeArray;
		size_t size = array.size();

		dstTypeArray.reserve(size);
		for (size_t i = 0; i < size; i++) {
			dstTypeArray.push_back(std::static_pointer_cast<DstType>(array[i]));
		}

		return dstTypeArray;
	}	


}