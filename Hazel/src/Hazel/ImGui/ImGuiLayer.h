#pragma once

#include "Hazel/Core/Layer.h"

#include "Hazel/Event/MouseEvent.h"
#include "Hazel/Event/ApplicationEvent.h"
#include "Hazel/Event/KeyEvent.h"


namespace Hazel {
	class HAZEL_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender();
		
		virtual void Begin() override;
		virtual void End() override;

		void SetDarkTheme();
	private:
		float m_Time = 0.0f;
	};
}