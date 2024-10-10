#pragma once

#include "nightpch.h"
#include "core.h"
#include "log/log.h"

namespace night
{

enum class EEventType : u8
{
	None = 0,

	WindowClose,
	WindowResize,
	WindowFocus,
	WindowLostFocus,
	WindowMoved,

	KeyPressed,
	KeyReleased,
	/* KeyTyped,*/

	MouseButtonPressed,
	MouseButtonReleased,
	MouseMotion,
	MouseWheel
};

enum EEventCategory : u8
{
	None = 0,
	EventCategoryApplication = BIT(0),
	EventCategoryInput = BIT(1),
	EventCategoryKeyboard = BIT(2),
	EventCategoryMouse = BIT(3),
	EventCategoryMouseButton = BIT(4)
};

#define EVENT_CLASS_TYPE(x)	static EEventType get_static_type() { return EEventType::x; } \
							virtual EEventType type() const override { return get_static_type(); } \
							virtual const char* name() const override { return #x; }

#define EVENT_CLASS_CATEGORY(x) virtual int category() const override { return x; }

struct Event
{
	virtual ~Event() = default;

	virtual EEventType type() const = 0;
	virtual s32 category() const = 0;
	virtual const s8* name() const = 0;
	virtual string to_string() const { return name(); }

	u8 is_in_category(EEventCategory category) { return this->category() & category; }

	void handled(u8 handled) { _handled = handled; }
	u8 handled() const { return _handled; }

private:

	u8 _handled = false;
};

struct EventDispatcher
{
	EventDispatcher(Event& event) : _event(event) {}

	template<typename T>
	u8 dispatch(auto fn)
	{
		if (_event.type() == T::get_static_type())
		{
			_event.handled(_event.handled() | fn(static_cast<T&>(_event)));
			return true;
		}

		return false;
	}

private:

	Event& _event;
};

}

//template<>
//struct fmt::formatter<light::Event> : fmt::formatter<light::string>
//{
//	auto format(light::Event& my, format_context& ctx) -> decltype(ctx.out())
//	{
//		return format_to(ctx.out(), "[my_type i={}]", my.to_string());
//	}
//};
