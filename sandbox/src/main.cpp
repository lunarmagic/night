
#include "nightpch.h"
#include "Sandbox/Sandbox.h"
#include "application/Application.h"

night::Application* night::create_application()
{
	return new night::Sandbox();
}