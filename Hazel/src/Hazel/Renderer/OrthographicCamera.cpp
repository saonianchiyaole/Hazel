#include "hzpch.h"
#include "Hazel/Renderer/OrthographicCamera.h"


#include "glm/gtc/matrix_transform.hpp"

namespace Hazel {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: Camera()
	{
		m_Left = left;
		m_Right = right;
		m_Bottom = bottom;
		m_Top = top;

		m_ViewMatrix = glm::mat4(1.0f);
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

		m_Type = ProjectionType::Orthographic;
	}

	void OrthographicCamera::RecalculateViewMatrix() {
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), m_Rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0f), m_Rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0f), m_Rotation.z, glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		RecalculateViewProjectionMatrix();

	}

	void OrthographicCamera::RecalculateProjectionMatrix()
	{
		m_ProjectionMatrix = glm::ortho(m_Left, m_Right, m_Bottom, m_Top);

		RecalculateViewProjectionMatrix();
	}

	void OrthographicCamera::RecalculateViewProjectionMatrix()
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		m_Left = left;
		m_Right = right;
		m_Bottom = bottom;
		m_Top = top;

		RecalculateProjectionMatrix();
	}


}