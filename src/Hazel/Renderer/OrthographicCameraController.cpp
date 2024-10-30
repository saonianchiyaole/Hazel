#include "hzpch.h"
#include "Hazel/Renderer/OrthographicCameraController.h"
#include <Hazel/KeyCodes.h>

namespace Hazel {



	OrthographicCameraController::OrthographicCameraController(Ref<OrthographicCamera> camera, float aspectRatio)
		: m_Camera(camera)
	{
		m_CameraMoveSpeed = 10.f;
		m_CameraRotateSpeed = 25.0f;
		m_ZoomLevel = 1.0f;
		m_AspectRatio = aspectRatio;

		InitalizeCamera();
	}

	void OrthographicCameraController::OnUpdate(float ts)
	{
		float deltaTime = ts;
		if (Input::IsKeyPressed(HZ_KEY_A)) {
			m_Camera->SetPosition({ m_Camera->GetPosition().x - m_CameraMoveSpeed * deltaTime, m_Camera->GetPosition().y, m_Camera->GetPosition().z });
		}
		else if (Input::IsKeyPressed(HZ_KEY_D)) {
			m_Camera->SetPosition({ m_Camera->GetPosition().x + m_CameraMoveSpeed * deltaTime, m_Camera->GetPosition().y, m_Camera->GetPosition().z });
		}

		if (Input::IsKeyPressed(HZ_KEY_W)) {
			m_Camera->SetPosition({ m_Camera->GetPosition().x, m_Camera->GetPosition().y + m_CameraMoveSpeed * deltaTime, m_Camera->GetPosition().z });
		}																				 
		else if (Input::IsKeyPressed(HZ_KEY_S)) {										 
			m_Camera->SetPosition({ m_Camera->GetPosition().x, m_Camera->GetPosition().y - m_CameraMoveSpeed * deltaTime, m_Camera->GetPosition().z });
		}

		if (Input::IsKeyPressed(HZ_KEY_Q)) {
			m_Camera->SetRotation(m_Camera->GetRotation() + m_CameraRotateSpeed * deltaTime);
		}
		if (Input::IsKeyPressed(HZ_KEY_E)) {
			m_Camera->SetRotation(m_Camera->GetRotation() - m_CameraRotateSpeed * deltaTime);
		}
	}
	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}
	
	void OrthographicCameraController::BindCamera(Ref<OrthographicCamera> camera)
	{
		m_Camera = camera;
		InitalizeCamera();
	}

	void OrthographicCameraController::UnbindCamera()
	{
		m_Camera = nullptr;
	}

	void OrthographicCameraController::InitalizeCamera()
	{
		m_Camera->SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}
	
	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.2f;
		m_ZoomLevel = std::min(std::max(m_ZoomLevel, 0.25f), 2.0f);
		m_Camera->SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera->SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	

}