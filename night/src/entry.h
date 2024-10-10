#pragma once

//#include "Log.h"

#ifdef NIGHT_PLATFORM_WINDOWS
extern night::Application* night::create_application();

#undef main // TODO: fix this

int main(int argc, char** argv)
{
	auto app = night::create_application();
	app->run();
	delete app;

	return 0;
}
#endif