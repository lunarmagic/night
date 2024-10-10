
#include "nightpch.h"
#include "ISurface.h"

namespace night
{
	ISurface::ISurface(const SurfaceParams& params)
		: _width(params.width)
		, _height(params.height)
	{
	}
}