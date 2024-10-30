#pragma once

#include "glm/glm.hpp"



namespace Hazel {

	class Camera {
	public:
		enum class CameraType {
			None = 0, Perspective, Orthographic
		};

	public:
		Camera();
		~Camera();
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; };
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; };
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
		void SetUp(const glm::vec3& up) { m_Up = up; RecalculateViewMatrix(); }
		void SetFront(const glm::vec3& front) { m_Front = front;  RecalculateViewMatrix(); }
		void SetRotation(const float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
		
		//For Ortho camera: @param1 : left @param2 : right @param3 : bottom @param4 : top
		//For perspective camera: @param1 : fov @param2 : aspectRatio @param3 : nearClip @param4 : farClip
		virtual void SetProjection(float param1, float param2, float param3, float param4) = 0;




		glm::vec3& GetPosition() { return m_Position; }
		float GetRotation() { return m_Rotation; }

		static Camera* Create(CameraType type);
		
		//For Ortho camera: @param1 : left @param2 : right @param3 : bottom @param4 : top
		//For perspective camera: @param1 : fov @param2 : aspectRatio @param3 : nearClip @param4 : farClip
		static Camera* Create(CameraType type, float param1, float param2, float param3, float param4);
	protected:
		virtual void RecalculateViewMatrix() = 0;
		virtual void RecalculateProjectionMatrix() = 0;
		virtual void RecalculateViewProjectionMatrix() = 0;


		glm::vec3 m_Up;
		glm::vec3 m_Front;
		glm::vec3 m_Position;
		float m_Rotation;

		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewProjectionMatrix;
		

	};

}
