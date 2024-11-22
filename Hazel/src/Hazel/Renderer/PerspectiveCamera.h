#pragma once
#include "Camera.h"


namespace Hazel {
	
	class PerspectiveCamera : public Camera {
	public:
		PerspectiveCamera(float param1, float param2, float param3, float param4);
		
		//For Ortho camera: @param1 : left @param2 : right @param3 : bottom @param4 : top
		//For perspective camera: @param1 : fov @param2 : aspectRatio @param3 : nearClip @param4 : farClip
		virtual void SetProjection(float param1, float param2, float param3, float param4) override;
	private:
		virtual void RecalculateViewMatrix() override;
		virtual void RecalculateProjectionMatrix() override;
		virtual void RecalculateViewProjectionMatrix() override;

	};
}