
#include "nightpch.h"
#include "AABB.h"
#include "math/math.h"

namespace night
{

void AABB::fit_around_point(const vec2& point)
{
	left = MIN(point.x, left);
	right = MAX(point.x, right);
	top = MIN(point.y, top);
	bottom = MAX(point.y, bottom);

	left = MIN(point.x, left);
	right = MAX(point.x, right);
	top = MIN(point.y, top);
	bottom = MAX(point.y, bottom);
}

}
