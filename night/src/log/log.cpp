
#include "nightpch.h"

#include <windows.h>
#include "log.h"

#ifdef NIGHT_ENABLE_LOGGING
namespace night { namespace debug {

	u8 _Log::_is_prev_cout_a_log = { false };
	s32 _Log::_debug_logging_mask = DEBUG_LOG_MASK_EVERYTHING;
	map<string, u32> _Log::_logged_messages = {};

	void _Log::_set_message_color(u16 id)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, id);
	}
}
}
#endif