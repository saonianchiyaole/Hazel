#pragma once


#include "Hazel/Input.h"

namespace Hazel {

	class WindowsInput : public Input {
	protected:
		inline virtual bool IsKeyPressedImpl(int keycode) override;
		
		inline virtual bool IsMouseButtonPressedImpl(int button) override;

		inline virtual float GetMouseXImpl() override;

		inline virtual float GetMouseYImpl() override;

		inline virtual std::pair<float, float> GetMousePosImpl() override;
	};
}