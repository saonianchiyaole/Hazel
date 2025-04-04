#include "hzpch.h"
#include "EditorCamera.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseButtonCodes.h"


namespace Hazel {




	Hazel::EditorCamera::EditorCamera(float fovy, float aspectRatio, float nearClip, float farClip)
		: Camera(ProjectionType::Perspective, fovy, aspectRatio, nearClip, farClip)
	{
		RecalculateViewMatrix();
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{

		if (Input::IsKeyPressed(HZ_KEY_LEFT_ALT)) {
			const glm::vec2& mousePos = { Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2 delta = (mousePos - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mousePos;

			if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_MIDDLE)) {
				MousePan(delta);
				RecalculateViewMatrix();
			}
			else if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_RIGHT)) {
				MouseRotate(delta);
				RecalculateViewMatrix();
			}
			else if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT)) {
				MouseZoom(delta.y);
				RecalculateViewMatrix();
			}
		}


	}


	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(m_Orientation, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(m_Orientation, glm::vec3(1.0f, 0.0f, 0.0f));
	}


	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(m_Orientation, glm::vec3(0.0f, 0.0f, -1.0f));

	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return m_Orientation;
	}

	void EditorCamera::RecalculateViewMatrix()
	{

		
		CalculateOrientation();

		m_Position = CalculatePosition();
		

		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(m_Orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);

		RecalculateViewProjectionMatrix();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
		//dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(EditorCamera::OnKeyPressed));
	}
	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		RecalculateViewMatrix();
		return false;
	}

	bool EditorCamera::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == HZ_KEY_X) {
			SetDefault();
		}
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	void EditorCamera::CalculateOrientation()
	{
		m_Orientation = glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}



}