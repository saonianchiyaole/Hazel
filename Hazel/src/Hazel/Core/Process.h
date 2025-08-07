#pragma once



namespace Hazel {


	class Process {

	public:


		virtual void ExcuteCommand() = 0;


	private:

		Ref<Process> s_Instance;

	};



}

