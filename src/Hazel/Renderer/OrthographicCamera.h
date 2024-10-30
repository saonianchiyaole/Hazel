#pragma once

#include "Hazel/Renderer/Camera.h"

namespace Hazel{

	class OrthographicCamera : public Camera{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);
		
		virtual void RecalculateViewMatrix() override;
		virtual void RecalculateProjectionMatrix() override;
		inline virtual void RecalculateViewProjectionMatrix() override;

		virtual void SetProjection(float left, float right, float bottom, float top) override;

		float GetLeft()		{ return m_Left; }
		float GetRight()	{ return m_Right; }
		float GetBottom()	{ return m_Bottom; }
		float GetTop()		{ return m_Top; }
	private:
		float m_Left;
		float m_Right;
		float m_Bottom;
		float m_Top;
	};

}
