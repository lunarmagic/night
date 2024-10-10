#pragma once

#include "core.h"
#include "color/Color.h"

namespace night
{

	struct Vertex
	{
		fvec4 point;
		Color8 color;
		fvec2 texture_coord;
	};

}