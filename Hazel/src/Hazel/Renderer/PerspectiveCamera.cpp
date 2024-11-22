#include "hzpch.h"

#include "Hazel/Renderer/PerspectiveCamera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace Hazel {
	
	PerspectiveCamera::PerspectiveCamera(float param1, float param2, float param3, float param4)
	{

		m_Fovy = param1; m_AspectRatio = param2; m_NearClip = param3; m_FarClip = param4;
		m_ViewMatrix = glm::mat4(1.0f);
		SetProjection(m_Fovy, m_AspectRatio, m_NearClip, m_FarClip);

		m_Type = ProjectionType::Perspective;
	}

	void PerspectiveCamera::SetProjection(float param1, float param2, float param3, float param4)
	{
		m_Fovy = param1; m_AspectRatio = param2; m_NearClip = param3; m_FarClip = param4;
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), m_Rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0f), m_Rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0f), m_Rotation.z, glm::vec3(0, 0, 1));
		m_ViewMatrix = glm::inverse(transform);
		RecalculateViewProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateProjectionMatrix()
	{
		m_ProjectionMatrix = glm::perspective(m_Fovy, m_AspectRatio, m_NearClip, m_FarClip);
		RecalculateViewProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateViewProjectionMatrix()
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}