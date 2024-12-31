#pragma once

#include <string>

namespace Hazel {

	class FileDialogs
	{
	public:
		
		friend class WindowsFileDialogs;
		
		// These return empty strings if cancelled
		static std::string OpenFile(const char* filter) { return s_Instance->OpenFileInternal(filter); };
		static std::string SaveFile(const char* filter) { return s_Instance->SaveFileInternal(filter); };

	protected:
		virtual std::string OpenFileInternal(const char* filter) = 0;
		virtual std::string SaveFileInternal(const char* filter) = 0;

		static FileDialogs* s_Instance;
	};


}