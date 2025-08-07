#pragma once

#include "Hazel/ImGui/ImGuiLayer.h"


namespace Hazel {


	class OpenGLImGuiLayer : public ImGuiLayer {

		OpenGLImGuiLayer() = default;
		~OpenGLImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender();

		virtual void Begin() override;
		virtual void End() override;

	};


}