#pragma once

#include "Event.h"

namespace night
{

struct MouseButtonEvent : public Event
{
	u32 button() const { return _button; }

	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

protected:

	MouseButtonEvent(u32 button) : _button(button) {}

private:

	u32 _button;
};

struct MouseButtonPressedEvent : public MouseButtonEvent
{
	MouseButtonPressedEvent(u32 button) : MouseButtonEvent(button) {}

	string to_string() const override
	{
		sstream ss;
		ss << "MouseButtonPressedEvent: " << button();
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonPressed)
};

struct MouseButtonReleasedEvent : public MouseButtonEvent
{
	MouseButtonReleasedEvent(u32 button) : MouseButtonEvent(button) {}

	string to_string() const override
	{
		sstream ss;
		ss << "MouseButtonReleasedEvent: " << button();
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonReleased)
};

struct MouseMotionEvent : public Event
{
	MouseMotionEvent(real x, real y) : _x(x), _y(y) {}

	real x() const { return _x; }
	real y() const { return _y; }

	string to_string() const override
	{
		sstream ss;
		ss << "MouseMotionEvent: " << x() << ", " << y();
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseMotion)
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:

	real _x;
	real _y;
};

struct MouseWheelEvent : public Event
{
	MouseWheelEvent(real dx, real dy) : _dx(dx), _dy(dy) {}

	real dx() const { return _dx; }
	real dy() const { return _dy; }

	string to_string() const override
	{
		sstream ss;
		ss << "MouseWheelEvent: " << dx() << ", " << dy();
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseWheel)
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:

	real _dx, _dy;
};

}

