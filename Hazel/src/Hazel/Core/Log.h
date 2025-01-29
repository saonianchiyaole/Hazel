#pragma once


#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"


namespace Hazel {
	class HAZEL_API Log
	{
	public:
		Log();
		~Log();

		static void Init();
		
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; };
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;


	};

}
//Core log macros
#define HZ_CORE_ERROR(...)   ::Hazel::Log::GetCoreLogger()->error(fmt::format(__VA_ARGS__))
#define HZ_CORE_WARN(...)    ::Hazel::Log::GetCoreLogger()->warn(fmt::format(__VA_ARGS__))
#define HZ_CORE_INFO(...)    ::Hazel::Log::GetCoreLogger()->info(fmt::format(__VA_ARGS__))
#define HZ_CORE_TRACE(...)   ::Hazel::Log::GetCoreLogger()->trace(fmt::format(__VA_ARGS__))
#define HZ_CORE_FATAL(...)   ::Hazel::Log::GetCoreLogger()->fatal(fmt::format(__VA_ARGS__))

// Client log macros

#define HZ_ERROR(...)		::Hazel::Log::GetClientLogger()->error(fmt::format(__VA_ARGS__))
#define HZ_WARN(...)		::Hazel::Log::GetClientLogger()->warn(fmt::format(__VA_ARGS__))
#define HZ_INFO(...)		::Hazel::Log::GetClientLogger()->info(fmt::format(__VA_ARGS__))
#define HZ_TRACE(...)		::Hazel::Log::GetClientLogger()->trace(fmt::format(__VA_ARGS__))
#define HZ_FATAL(...)		::Hazel::Log::GetClientLogger()->fatal(fmt::format(__VA_ARGS__))