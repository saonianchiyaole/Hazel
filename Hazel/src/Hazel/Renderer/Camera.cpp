#include "hzpch.h"
#include "Hazel/Renderer/Camera.h"

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/PerspectiveCamera.h"
#include <glm/gtx/quaternion.hpp>

namespace Hazel {

	Camera::Camera() :m_Position(0.0f), m_Rotation(0.0f), m_ViewMatrix(1.0f) {
		m_Type = ProjectionType::Perspective;
		m_Fovy = 45.f;
		m_Fovx = 45.f;
		m_ZoomLevel = 1.0f;
		m_AspectRatio = 1280.f / 720.f;
		m_NearClip = 0.001f;
		m_FarClip = 10000.f;
		RecalculateProjectionMatrix();
	}

	Camera::Camera(ProjectionType type, float param1, float param2, float param3, float param4)
	{
		m_Type = type;
		m_AspectRatio = param2;
		m_NearClip = param3;
		m_FarClip = param4;
		if (type == ProjectionType::Perspective) {
			m_Fovy = param1;
			m_Fovx = 45.f;
			m_ZoomLevel = 1.0f;
			
			
		}
		else if (type == ProjectionType::Orthographic) {
			m_Fovy = 45.0f;
			m_Fovx = 45.f;
			m_ZoomLevel = param1;
		}
		RecalculateProjectionMatrix();
	}

	Camera::~Camera()
	{

	}

	void Camera::SetOrthographicProjection(float zoomLevel, float aspectRatio, float nearClip, float farClip)
	{
		m_Type = ProjectionType::Orthographic;
		m_ZoomLevel = zoomLevel; m_AspectRatio = aspectRatio;
		m_NearClip = nearClip; m_FarClip = farClip;
		RecalculateProjectionMatrix();
	}

	void Camera::SetPerspectiveProjection(float fovy, float aspectRatio, float nearClip, float farClip)
	{
		m_Type = ProjectionType::Perspective;
		m_Fovy = fovy; m_AspectRatio = aspectRatio;
		m_NearClip = nearClip; m_FarClip = farClip;
		RecalculateProjectionMatrix();
	}

	void Camera::SetProjection(float param1, float param2, float param3, float param4)
	{
		if (m_Type == ProjectionType::Perspective) {
			m_Fovy = param1; m_AspectRatio = param2;
			m_NearClip = param3; m_FarClip = param4;
		}
		else if (m_Type == ProjectionType::Orthographic) {
			m_ZoomLevel = param1; m_AspectRatio = param2;
			m_NearClip = param3; m_FarClip = param4;
		}
		RecalculateProjectionMatrix();
	}

	Camera* Camera::Create(ProjectionType type, float param1, float param2, float param3, float param4) {
		switch (type)
		{
		case Hazel::Camera::ProjectionType::None:
			HZ_CORE_ASSERT(false, "Please specific the camera type!");
			break;
		case Hazel::Camera::ProjectionType::Perspective:  // Not define yet
			return new PerspectiveCamera(param1, param2, param3, param4);
		case Hazel::Camera::ProjectionType::Orthographic:
			return new OrthographicCamera(param1, param2, param3, param4);
		}
	}

	Camera::operator OrthographicCamera()
	{
		return *(OrthographicCamera*)this;
	}

	void Camera::RecalculateViewMatrix()
	{
		glm::mat4 rotationMat = glm::toMat4(glm::quat(m_Rotation));
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * rotationMat;


		m_ViewMatrix = glm::inverse(transform);
		RecalculateViewProjectionMatrix();
	}

	void Camera::RecalculateProjectionMatrix()
	{
		if (m_Type == ProjectionType::Perspective) {
			m_ProjectionMatrix = glm::perspective(glm::radians(m_Fovy), m_AspectRatio, m_NearClip, m_FarClip);
		}
		else if (m_Type == ProjectionType::Orthographic) {
			m_ProjectionMatrix = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		}

		RecalculateViewProjectionMatrix();
	}

	void Camera::RecalculateViewProjectionMatrix()
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}


}