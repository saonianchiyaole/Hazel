#pragma once

#include "Hazel/Core.h"
#include "Hazel/Event/Event.h"
#include "Window.h"

namespace Hazel {

	class HAZEL_API Application
	{

	public :
		Application();
		virtual ~Application();

		void Run();
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		
	};
	// To be define in Client
	Application* CreateApplication();
}

