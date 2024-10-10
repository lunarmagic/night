#pragma once

#include "core.h"
#include "ISurface.h"

namespace night
{
	struct WindowSDL;

	struct /*NIGHT_API*/ TextureParams
	{
		s32 width{};
		s32 height{};
		ref<ISurface> surface{};
		string path{ "" };
	};

	struct NIGHT_API ITexture
	{
		ITexture() = default;
		ITexture(const TextureParams& params);

		s32 width() const { return _width; };
		s32 height() const { return _height; };

	//protected:

	//	void width(s32 w) { _width = w; }
	//	void height(s32 h) { _height = h; }

	private:

		s32 _width{ 0 };
		s32 _height{ 0 };
	};

}