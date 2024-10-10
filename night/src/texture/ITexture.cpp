
#include "nightpch.h"
#include "ITexture.h"

namespace night
{
	ITexture::ITexture(const TextureParams& params)
	{
		_width = params.width;
		_height = params.height;
	}
}
