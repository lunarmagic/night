
#include "nightpch.h"

#include <windows.h>
#include "log.h"

#ifdef NIGHT_ENABLE_LOGGING
namespace night { namespace debug {

	//s32 _debug_logging_mask = DEBUG_LOG_MASK_EVERYTHING;

	void _set_message_color(u16 id)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, id);
	}
}
}
#endif