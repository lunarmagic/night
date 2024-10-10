#pragma once

#include "core.h"
#include "color/Color.h"

namespace night
{

	struct SurfaceParams
	{
		s32 width{};
		s32 height{};
		string path{ "" };
	};

	struct NIGHT_API ISurface
	{
		ISurface(const SurfaceParams& params);

		s32 width() const { return _width; }
		s32 height() const { return _height; }

		virtual Color8* pixels() const = 0; 

	private:

		s32 _width{};
		s32 _height{};
	};

}