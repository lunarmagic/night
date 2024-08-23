
#include "nightpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace night
{
	std::shared_ptr<spdlog::logger> Log::_coreLogger;
	std::shared_ptr<spdlog::logger> Log::_clientLogger;

	void Log::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		_coreLogger = spdlog::stdout_color_mt("Core");
		_coreLogger->set_level(spdlog::level::trace);
		_clientLogger = spdlog::stdout_color_mt("Client");
		_clientLogger->set_level(spdlog::level::trace);
	}
}