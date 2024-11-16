#include "hzpch.h"

#include "Hazel/Renderer/PerspectiveCamera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace Hazel {
	
	PerspectiveCamera::PerspectiveCamera(float param1, float param2, float param3, float param4)
	{

		m_Fovy = param1; m_AspectRatio = param2; m_ZNear = param3; m_ZFar = param4;
		m_ViewMatrix = glm::mat4(1.0f);
		SetProjection(m_Fovy, m_AspectRatio, m_ZNear, m_ZFar);
	}

	void PerspectiveCamera::SetProjection(float param1, float param2, float param3, float param4)
	{
		m_Fovy = param1; m_AspectRatio = param2; m_ZNear = param3; m_ZFar = param4;
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
								glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.f, 0.f, 1.f));
		m_ViewMatrix = glm::inverse(transform);
		RecalculateViewProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateProjectionMatrix()
	{
		m_ProjectionMatrix = glm::perspective(m_Fovy, m_AspectRatio, m_ZNear, m_ZFar);
		RecalculateViewProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateViewProjectionMatrix()
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}