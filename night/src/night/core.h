#pragma once

#ifdef NIGHT_PLATFORM_WINDOWS
	#ifdef NIGHT_BUILD_DLL
		#define NIGHT_API __declspec(dllexport)
	#else
		#define NIGHT_API __declspec(dllimport)
#endif
#else
	#error NIGHT ONLY SUPPORTS WINDOWS
#endif