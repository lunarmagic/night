
#include "nightpch.h"
#include "utility.h"
#include "application/Application.h"

namespace night
{

	IWindow& utility::window()
	{
		return Application::get().window();
	}

	IRenderer& utility::renderer()
	{
		return Application::get().window().renderer();
	}

}


