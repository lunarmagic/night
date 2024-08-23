#pragma once

#include "core.h"
#include "spdlog/spdlog.h"

namespace night
{
	class NIGHT_API Log
	{
		static std::shared_ptr<spdlog::logger> _coreLogger;
		static std::shared_ptr<spdlog::logger> _clientLogger;
	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& core_logger() { return _coreLogger; };
		inline static std::shared_ptr<spdlog::logger>& client_logger() { return _clientLogger; };
	};
}

// core log macros
#define NIGHT_CORE_TRACE(...)		::night::Log::core_logger()->trace(__VA_ARGS__)
#define NIGHT_CORE_INFO(...)		::night::Log::core_logger()->info(__VA_ARGS__)
#define NIGHT_CORE_WARNING(...)		::night::Log::core_logger()->warn(__VA_ARGS__)
#define NIGHT_CORE_ERROR(...)		::night::Log::core_logger()->error(__VA_ARGS__)
#define NIGHT_CORE_FATAL(...)		::night::Log::core_logger()->critical(__VA_ARGS__)

// client log macros
#define NIGHT_CLIENT_TRACE(...)		::night::Log::client_logger()->trace(__VA_ARGS__)
#define NIGHT_CLIENT_INFO(...)		::night::Log::client_logger()->info(__VA_ARGS__)
#define NIGHT_CLIENT_WARNING(...)	::night::Log::client_logger()->warn(__VA_ARGS__)
#define NIGHT_CLIENT_ERROR(...)		::night::Log::client_logger()->error(__VA_ARGS__)
#define NIGHT_CLIENT_FATAL(...)		::night::Log::client_logger()->critical(__VA_ARGS__)

//#undef TRACE
//#undef INFO
//#undef WARNING
//#undef ERROR
//#undef FATAL
//
//#ifdef LIGHT_ENABLE_LOGGING
//#ifdef LIGHT_CORE
//#define TRACE(...)		LIGHT_CORE_TRACE(__VA_ARGS__)	
//#define INFO(...)		LIGHT_CORE_INFO(__VA_ARGS__)		
//#define WARNING(...)	LIGHT_CORE_WARNING(__VA_ARGS__)	
//#define ERROR(...)		LIGHT_CORE_ERROR(__VA_ARGS__)	
//#define FATAL(...)		LIGHT_CORE_FATAL(__VA_ARGS__)
//#else
//#define TRACE(...)		LIGHT_CLIENT_TRACE(__VA_ARGS__)	
//#define INFO(...)		LIGHT_CLIENT_INFO(__VA_ARGS__)		
//#define WARNING(...)	LIGHT_CLIENT_WARNING(__VA_ARGS__)	
//#define ERROR(...)		LIGHT_CLIENT_ERROR(__VA_ARGS__)	
//#define FATAL(...)		LIGHT_CLIENT_FATAL(__VA_ARGS__)
//#endif
//#else
//#define TRACE(...)
//#define INFO(...)
//#define WARNING(...)
//#define ERROR(...)
//#define FATAL(...)
//#endif

