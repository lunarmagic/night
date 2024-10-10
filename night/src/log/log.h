#pragma once

#include "core.h"
//#include "utility.h"
#include <iostream>

#undef LOG
#undef WARNING
#undef ERROR

// TODO: add log_level and define SET_LOG_LEVEL
// TODO: add PRINT and formatting to log

#define LOG_MAX_MESSAGES 5

#define DEBUG_LOG_MASK_OFF 0
#define DEBUG_LOG_MASK_PRINT BIT(1)
#define DEBUG_LOG_MASK_ERROR BIT(2)
#define DEBUG_LOG_MASK_WARNING BIT(3)
#define DEBUG_LOG_MASK_TRACE BIT(4)
#define DEBUG_LOG_MASK_EVERYTHING S32_MAX

namespace night { namespace debug {

#ifdef NIGHT_ENABLE_LOGGING

	static u8 _is_prev_cout_a_log{ false };
	static map<string, u32> _logged_messages;

	//enum class EDebugLoggingMask : u32
	//{
	//	OFF = 0,
	//	PRINT = BIT(1),
	//	ERROR = BIT(2),
	//	WARNING = BIT(3),
	//	TRACE = BIT(4),
	//	EVERYTHING = 0xFFFFFFFF,
	//};

	//static s32 _debug_logging_mask{ DEBUG_LOG_MASK_EVERYTHING };

	void NIGHT_API _set_message_color(u16 id);

	template<typename T>
	static void _log(sstream& out_stream, T&& t)
	{
		out_stream << t /*<< '\n'*/;
	}

	template<typename T, typename... Args>
	static void _log(sstream& out_stream, T&& t, Args&&... args)
	{
		out_stream << t;
		_log(out_stream, std::forward<Args&&>(args)...);
	}

	template<typename... Args>
	static void _log(Args&&... args)
	{
		if (!_is_prev_cout_a_log)
		{
			std::cout << '\n';
		}

		_set_message_color(7);

		sstream stream;
		//stream << "Log: ";
		_log(stream, std::forward<Args&&>(args)...);
		std::cout << stream.str();
		_is_prev_cout_a_log = true;
	}

	template<typename... Args>
	static void _message(const char* type, const char* file, s32 line, Args&&... args)
	{
		sstream stream;
		string key = string(type) + string(file) + to_string(line);

		auto i = _logged_messages.find(key);
		if (i != _logged_messages.end())
		{
			if ((*i).second >= LOG_MAX_MESSAGES)
			{
				return;
			}

			(*i).second++;
			stream << "(" << (*i).second << ") ";
		}
		else
		{
			_logged_messages.insert({ key, 1 });
		}

		stream << type << "\n  File: " << file << ",\n  Line: " << line << ",\n  Message: ";
		_log(stream, std::forward<Args&&>(args)...);
		std::cout << '\n' << stream.str();
	}

	template<typename... Args>
	static void _trace(Args&&... args)
	{
		//if (!(_debug_logging_mask & DEBUG_LOG_MASK_TRACE))
		//{
		//	return;
		//}

		_log(args...);
		_log("\n");
	}

	template<typename T> string _print_format(T& t);

	template<typename T>
	static void _print(T& t, const char* var_name)
	{
		//if (!(_debug_logging_mask & DEBUG_LOG_MASK_PRINT))
		//{
		//	return;
		//}

		_log("\n");
		_log("Print ", var_name, ": ");
		_log(_print_format(t));
		_log("\n");
	}

	template<typename... Args>
	static void _warning(const char* file, s32 line, Args&&... args)
	{
		//if (!(_debug_logging_mask & DEBUG_LOG_MASK_WARNING))
		//{
		//	return;
		//}

		_set_message_color(14);
		_message("Warning!", file, line, std::forward<Args&&>(args)...);
	}

	template<typename... Args>
	static void _error(const char* file, s32 line, Args&&... args)
	{
		//if (!(_debug_logging_mask & DEBUG_LOG_MASK_ERROR))
		//{
		//	return;
		//}

		_set_message_color(12);
		_message("Error!", file, line, std::forward<Args&&>(args)...);
	}
}
}

#define PRINT(x) ::night::debug::_print(x, #x)
#define TRACE(...) ::night::debug::_trace(##__VA_ARGS__)
#define WARNING(...) ::night::debug::_warning(__FILE__, __LINE__, ##__VA_ARGS__)
#define ERROR(...) ::night::debug::_error(__FILE__, __LINE__, ##__VA_ARGS__); __debugbreak();
#else
//#define DEBUG_LOG_MASK 0
//#define DEBUG_LOG_SET_MASK(x)
#define PRINT(x)
#define TRACE(...)
#define WARNING(...)
#define ERROR(...)
#endif

//#define DEBUG_LOG_MASK ::night::debug::_debug_logging_mask
//#define DEBUG_LOG_SET_MASK(x) ::night::debug::_debug_logging_mask = x

#undef ASSERT
#ifdef NIGHT_DEBUG
#define ASSERT(x) { if(!(x)) { __debugbreak(); } }
#else
#define ASSERT(x)
#endif