#pragma once

#include "Event.h"

namespace night
{

struct KeyEvent : public Event
{
	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	u32 keycode() const { return _keycode; }

protected:

	KeyEvent(u32 keycode) : _keycode(keycode) {}

private:

	u32 _keycode; // TODO create Keycode class.
};

struct KeyPressedEvent : public KeyEvent
{
	KeyPressedEvent(u32 keycode, u8 isRepeat) : KeyEvent(keycode), _isRepeat(isRepeat) {}

	string to_string() const override
	{
		sstream ss;
		ss << "KeyPressedEvent: " << keycode() << " (repeat = " << isRepeat() << ")";
		return ss.str();
	}

	u8 isRepeat() const { return _isRepeat; }
	EVENT_CLASS_TYPE(KeyPressed)

private:

	u8 _isRepeat;
};

struct KeyReleasedEvent : public KeyEvent
{
	KeyReleasedEvent(u32 keycode) : KeyEvent(keycode) {}

	string to_string() const override
	{
		sstream ss;
		ss << "KeyReleasedEvent: " << keycode();
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased)
};

//class KeyTypedEvent : public KeyEvent
//{
//public:
//
//	KeyTypedEvent(u32 keycode) : KeyEvent(keycode) {}
//
//	string to_string() const override
//	{
//		sstream ss;
//		ss << "KeyTypedEvent: " << keycode();
//		return ss.str();
//	}
//
//	EVENT_CLASS_TYPE(KeyTyped)
//};

}

