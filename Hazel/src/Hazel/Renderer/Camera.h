#pragma once

#include "glm/glm.hpp"


namespace Hazel {

	class OrthographicCamera;
	class PerspectiveCamera;


	class Camera {
		friend class SceneSerializer;
	public:
		enum class ProjectionType {
			Perspective = 0, Orthographic = 1, None = 3
		};

	public:
		Camera();
		Camera(ProjectionType type, float param1, float param2, float param3, float param4);
		virtual ~Camera();

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; };
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; };
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		const ProjectionType GetProjectionType() { return m_Type; }

		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
		void SetUp(const glm::vec3& up) { m_Up = up; RecalculateViewMatrix(); }
		void SetFront(const glm::vec3& front) { m_Front = front;  RecalculateViewMatrix(); }
		void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
		void SetTransform(const glm::vec3& position, const glm::vec3& rotation)
		{
			m_Position = position; m_Rotation = rotation; RecalculateViewMatrix();
		}

		void SetOrthographicProjection(float zoomLevel, float aspectRatio, float nearClip, float farClip);
		void SetPerspectiveProjection(float fovy, float aspectRatio, float nearClip, float farClip);
		void SetProjectionType(ProjectionType type) { m_Type = type; RecalculateProjectionMatrix();}

		void SetFovy(float fovy)				{ m_Fovy = fovy;				RecalculateProjectionMatrix(); }
		void SetFovx(float fovx)				{ m_Fovx = fovx;				RecalculateProjectionMatrix(); }
		void SetAspectRatio(float aspectRatio)	{ m_AspectRatio = aspectRatio;	RecalculateProjectionMatrix(); }
		void SetZoomLevel(float zoomLevel)		{ m_ZoomLevel = zoomLevel;		RecalculateProjectionMatrix(); }
		void SetNearClip(float nearClip)		{ m_NearClip = nearClip;		RecalculateProjectionMatrix(); }
		void SetFarClip(float farClip)			{ m_FarClip = farClip;			RecalculateProjectionMatrix(); }



		//For Ortho camera: @param1 : left @param2 : right @param3 : bottom @param4 : top
		//For perspective camera: @param1 : fov @param2 : aspectRatio @param3 : nearClip @param4 : farClip
		virtual void SetProjection(float param1, float param2, float param3, float param4);

		glm::vec3& GetPosition() { return m_Position; }
		inline glm::vec3 GetRotation() { return m_Rotation; }
		inline float GetFovy() { return m_Fovy;}
		inline float GetFovx() { return m_Fovx;}
		inline float GetAspectRatio() { return m_AspectRatio;}
		inline float GetZoomLevel() { return m_ZoomLevel;}
		inline float GetNearClip() { return m_NearClip; }
		inline float GetFarClip() { return m_FarClip;}

		float GetLeft() {
			HZ_CORE_ASSERT(this->m_Type == ProjectionType::Orthographic, "Camera's projection type is not orthographic!");
			return -m_AspectRatio * m_ZoomLevel;
		}
		float GetRight() {
			HZ_CORE_ASSERT(this->m_Type == ProjectionType::Orthographic, "Camera's projection type is not orthographic!");
			return m_AspectRatio * m_ZoomLevel;
		}
		float GetBottom() {
			HZ_CORE_ASSERT(this->m_Type == ProjectionType::Orthographic, "Camera's projection type is not orthographic!");
			return -m_ZoomLevel;
		}
		float GetTop() {
			HZ_CORE_ASSERT(this->m_Type == ProjectionType::Orthographic, "Camera's projection type is not orthographic!");
			return m_ZoomLevel;
		}



		static Camera* Create(ProjectionType type);

		//For Ortho camera: @param1 : left @param2 : right @param3 : bottom @param4 : top
		//For perspective camera: @param1 : fov @param2 : aspectRatio @param3 : nearClip @param4 : farClip
		static Camera* Create(ProjectionType type, float param1, float param2, float param3, float param4);

		operator OrthographicCamera ();

	protected:
		virtual void RecalculateViewMatrix();
		virtual void RecalculateProjectionMatrix();
		virtual void RecalculateViewProjectionMatrix();


		glm::vec3 m_Up;
		glm::vec3 m_Front;
		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation = { 0.f, 0.f, 0.f };

		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		ProjectionType m_Type;

		float m_Fovy;
		float m_Fovx;
		float m_ZoomLevel;
		float m_AspectRatio;
		float m_NearClip;
		float m_FarClip;

	};

}
