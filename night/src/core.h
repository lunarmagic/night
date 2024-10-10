#pragma once

#ifdef NIGHT_PLATFORM_WINDOWS
	#ifdef NIGHT_BUILD_DLL
		#define NIGHT_API __declspec(dllexport)
	#else
		#define NIGHT_API __declspec(dllimport)
	#endif
#else
	#error Light only supports Windows!
#endif