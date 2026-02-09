#pragma once


namespace Hazel {


	namespace Utils{

		void SetCurrentThreadName(const std::string& name);
		void SetThreadName(HANDLE threadHANDLE, const std::string& name);
	}


}