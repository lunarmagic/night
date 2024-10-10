#pragma once

//#include "utility.h"
#include "core.h"

namespace night
{

struct NIGHT_API AABB
{
	real left{ INFINITY };
	real right{ -INFINITY };
	real top{ INFINITY };
	real bottom{ -INFINITY };

	void fit_around_point(const vec2& point);
	real width() const { return right - left; }
	real height() const { return bottom - top; }
};

}
