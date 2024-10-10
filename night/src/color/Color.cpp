
#include "nightpch.h"
#include "Color.h"
#include "math/math.h"

namespace night
{
	Color::Color(const Color8& color8)
	{
		r = (real)color8.r / 255.0f;
		g = (real)color8.g / 255.0f;
		b = (real)color8.b / 255.0f;
		a = (real)color8.a / 255.0f;
	}

	Color Color::rainbow(real t)
	{
		s32 normalized = s32(t * 256 * 6);
		s32 region = normalized / 256;
		s32 x = normalized % 256;

		uint8_t r = 0, g = 0, b = 0;

		switch (region)
		{
		case 0: r = 255; g = 0;   b = 0;   g += x; break;
		case 1: r = 255; g = 255; b = 0;   r -= x; break;
		case 2: r = 0;   g = 255; b = 0;   b += x; break;
		case 3: r = 0;   g = 255; b = 255; g -= x; break;
		case 4: r = 0;   g = 0;   b = 255; r += x; break;
		case 5: r = 255; g = 0;   b = 255; b -= x; break;
		}

		u32 color = r + (g << 8) + (b << 16);
		Color8 color8 = *(Color8*)&color;
		color8.a = 255;
		return Color(color8);
	}
	Color Color::lerp(const Color& a, const Color& b, real t)
	{
		return Color(night::lerp(a.r, b.r, t), night::lerp(a.g, b.g, t), night::lerp(a.b, b.b, t), night::lerp(a.a, b.a, t));
	}
}

