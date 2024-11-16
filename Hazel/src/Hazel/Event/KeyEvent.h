#pragma once


#include "Event.h"


namespace Hazel {
	class HAZEL_API KeyEvent : public Event {
	public :
		inline int GetKeyCode() const { return m_KeyCode; }
		inline int GetScanCode() const { return m_ScanCode; }
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(int keycode, int scancode = 0 ): m_KeyCode(keycode), m_ScanCode(scancode){}
		int m_KeyCode;
		int m_ScanCode;
	};

	class HAZEL_API KeyPressedEvent : public KeyEvent {
	public :
		KeyPressedEvent(int keycode, int repeatCount, int scancode = 0):KeyEvent(keycode, scancode), m_RepeatCount(repeatCount){}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << "(" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};
	class HAZEL_API KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(int keycode, int scancode = 0): KeyEvent(keycode, scancode) {}
		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class HAZEL_API KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const int keycode, int scancode = 0)
			: KeyEvent(keycode, scancode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}