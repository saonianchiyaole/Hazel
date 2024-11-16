#include "hzpch.h"
#include "Hazel/Renderer/Camera.h"

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"


#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/PerspectiveCamera.h"

namespace Hazel {

	Camera::Camera() :m_Position(0.0f), m_Rotation(0.0f), m_ViewMatrix(1.0f){
		type = CameraType::Orthographic;
	}

	Camera::~Camera()
	{

	}

	Camera* Camera::Create(CameraType type, float param1, float param2, float param3, float param4){
		switch (type)
		{
		case Hazel::Camera::CameraType::None:
			HZ_CORE_ASSERT(false, "Please specific the camera type!");
			break;
		case Hazel::Camera::CameraType::Perspective:  // Not define yet
			return new PerspectiveCamera(param1, param2, param3, param4);
		case Hazel::Camera::CameraType::Orthographic:
			return new OrthographicCamera(param1, param2, param3, param4);
		}


	}

	Camera::operator OrthographicCamera()
	{
		return *(OrthographicCamera*)this;
	}


}