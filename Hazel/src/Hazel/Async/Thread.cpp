#include "hzpch.h"


#include "Hazel/Async/Thread.h"

#ifdef WIN32
	#include <Windows.h>
	#include <processthreadsapi.h>
#endif // WIN32


namespace Hazel {


	namespace Utils {

		void SetCurrentThreadName(const std::string& name) {

#ifdef WIN32

			wchar_t wname[256] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wname, sizeof(wname) / sizeof(wchar_t));
			SetThreadDescription(GetCurrentThread(), wname);

#endif // WIN32

		}

		void SetThreadName(HANDLE threadHandle, const std::string& name) {


#ifdef WIN32

			wchar_t wname[256] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wname, sizeof(wname) / sizeof(wchar_t));
			SetThreadDescription(threadHandle, wname);

#endif // WIN32

		}


	}


}