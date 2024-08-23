#pragma once

#include "Log.h"

#ifdef NIGHT_PLATFORM_WINDOWS

extern night::Application* night::create_application();

int main(int argc, char** argv)
{
	night::Log::init();
	NIGHT_CORE_INFO("init log");
	NIGHT_CORE_FATAL("d log");

	auto app = night::create_application();
	app->run();
	delete app;
}
#endif