#pragma once

#include "Hazel/Layer.h"

#include "Hazel/Event/MouseEvent.h"
#include "Hazel/Event/ApplicationEvent.h"
#include "Hazel/Event/KeyEvent.h"


namespace Hazel {
	class HAZEL_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnUpdate();
		void OnEvent(Event& event) override;


	private:
		//Kinds of mouse event
		bool OnMouseButtonPressedEvent		(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent		(MouseButtonReleasedEvent& e);
		bool OnMouseMovedEvent				(MouseMovedEvent& e);
		bool OnMouseScrolledEvent			(MouseScrolledEvent& e);
		//Keyboard event
		bool OnKeyPressedEvent				(KeyPressedEvent& e);
		bool OnKeyReleasedEvent				(KeyReleasedEvent& e);
		bool OnKeyTypedEvent				(KeyTypedEvent& e);
		//Application event
		bool OnWindowResizedEvent			(WindowResizeEvent& e);
		

	private:
		float m_Time;
	};
}