#pragma once

#include "Hazel/Renderer/OrthographicCamera.h"

#include "Hazel/Event/MouseEvent.h"
#include "Hazel/Event/ApplicationEvent.h"

#include "Hazel/Core/Input.h"

namespace Hazel {

	class OrthographicCameraController {
	public:
		OrthographicCameraController(Ref<OrthographicCamera> camera, float aspectRatio);
	
		void OnUpdate(float ts);
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
		void OnEvent(Event& e);
		void SetAspectRatio(float ratio);


		void BindCamera(Ref<OrthographicCamera> camera);
		void UnbindCamera();

		void InitalizeCamera();

	private:
		float m_AspectRatio;
		float m_ZoomLevel;

		float m_CameraMoveSpeed;
		float m_CameraRotateSpeed;

		Ref<OrthographicCamera> m_Camera;
	
	};
}