
#include <night.h>

class Sandbox : public night::Application
{
public:
	Sandbox() {};
	~Sandbox() {};
};

night::Application* night::create_application()
{
	return new Sandbox;
}

//int main()
//{
//	Sandbox* app = new Sandbox;
//	app->run();
//	delete app;
//}