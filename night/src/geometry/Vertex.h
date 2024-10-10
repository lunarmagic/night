#pragma once

#include "core.h"
#include "color/Color.h"

namespace night
{

struct /*NIGHT_API*/ Vertex
{
	fvec2 point;
	Color8 color;
	fvec2 texture_coord;

	//vec2 point;

	//Color8 color;

	//vec2 texture_coord;
};

}