#pragma once

#include "core.h"
#include "window/IWindow.h"
#include "renderer/IRenderer.h"

namespace night
{

	struct NIGHT_API utility
	{
		static IWindow& window();
		static IRenderer& renderer();
	};

}