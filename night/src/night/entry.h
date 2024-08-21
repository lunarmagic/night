#pragma once

#ifdef NIGHT_PLATFORM_WINDOWS

extern night::Application* night::create_application();

int main(int argc, char** argv)
{
	auto app = night::create_application();
	app->run();
	delete app;
}
#endif