#pragma once


#include "Hazel/Utils/PlatformUtils.h"
#include "Hazel/Core/Process.h"

namespace Hazel {

	class WindowsFileDialogs : public FileDialogs {
	protected:
		virtual std::string OpenFileInternal(const char* fileter) override;
		virtual std::string SaveFileInternal(const char* fileter) override;
	};


	class WindowsProcess : public Process {



	};


}