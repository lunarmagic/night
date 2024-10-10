#pragma once

#include "Event.h"

namespace night
{

struct WindowResizeEvent : public Event
{
	WindowResizeEvent(u32 width, u32 height) : _width(width), _height(height) {}

	u32 width() const { return _width; }
	u32 height() const { return _height; }

	string to_string() const override
	{
		sstream ss;
		ss << "WindowResizeEvent: " << width() << ", " << height();
		return ss.str();
	}

	EVENT_CLASS_TYPE(WindowResize)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)

private:

	u32 _width;
	u32 _height;
};

struct WindowCloseEvent : public Event
{
	WindowCloseEvent() = default;

	EVENT_CLASS_TYPE(WindowClose)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

}