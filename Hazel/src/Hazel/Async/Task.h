#pragma once


#include <functional>

class Task {



	void operator()() {



	}


	std::function<void()> m_Func;	


};
