#pragma once

//#include "utility.h"
#include "core.h"

#define COLOR_ZERO	night::Color(0.0f, 0.0f, 0.0f, 0.0f)
#define INVISIBLE	night::Color(1.0f, 1.0f, 1.0f, 0.0f)
#define LIGHT		night::Color(0.8f, 0.8f, 0.8f, 1.0f)
#define BLACK		night::Color(0.0f, 0.0f, 0.0f, 1.0f)
#define WHITE		night::Color(1.0f, 1.0f, 1.0f, 1.0f)
#define DARK_GREY	night::Color(0.25f, 0.25f, 0.25f, 1.0f)
#define GREY		night::Color(0.5f, 0.5f, 0.5f, 1.0f)
#define RED			night::Color(1.0f, 0.0f, 0.0f, 1.0f)
#define GREEN		night::Color(0.0f, 1.0f, 0.0f, 1.0f)
#define BLUE		night::Color(0.0f, 0.0f, 1.0f, 1.0f)
#define CYAN		night::Color(0.0f, 1.0f, 1.0f, 1.0f)
#define YELLOW		night::Color(1.0f, 1.0f, 0.0f, 1.0f)
#define PURPLE		night::Color(1.0f, 0.0f, 1.0f, 1.0f)
#define ORANGE		night::Color(1.0f, 0.5f, 0.0f, 1.0f)
#define PINK		night::Color(1.0f, 0.4f, 0.7f, 1.0f)
#define BROWN		night::Color(0.57f, 0.31f, 0.08, 1.0f)

namespace night
{

	struct Color8;

	struct NIGHT_API Color
	{
		real r{ 1.0f };
		real g{ 1.0f };
		real b{ 1.0f };
		real a{ 1.0f };

		Color() = default;
		Color(const Color& other) = default;
		Color(real r, real g, real b, real a = 1.0f) : r(r), g(g), b(b), a(a) {}
		Color(const Color8& color8);

		inline const Color operator* (const real& f) const
		{
			return Color((r * f), (g * f), (b * f), a);
		}

		inline const Color operator/ (const real& f) const
		{
			return Color((r / f), (g / f), (b / f), a);
		}

		inline const Color operator*= (const real& f)
		{
			r = (r * f);
			g = (g * f);
			b = (b * f);
			return *this;
		}

		inline const Color operator/= (const real& f)
		{
			r = (r / f);
			g = (g / f);
			b = (b / f);
			return *this;
		}

		static Color random()
		{
			return { ::night::random(1.0), ::night::random(1.0), ::night::random(1.0), 1.0f };
		}

		static Color rainbow(real t);

		static Color lerp(const Color& a, const Color& b, real t);
	};


	//inline Color lerp(const Color& a, const Color& b, real t)
	//{
	//	return Color(lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t), lerp(a.a, b.a, t));
	//}
	//
	//inline Color rainbow(real t)
	//{
	//	int normalized = int(t * 256 * 6);
	//	int region = normalized / 256;
	//	int x = normalized % 256;
	//
	//	uint8_t r = 0, g = 0, b = 0;
	//	switch (region)
	//	{
	//	case 0: r = 255; g = 0;   b = 0;   g += x; break;
	//	case 1: r = 255; g = 255; b = 0;   r -= x; break;
	//	case 2: r = 0;   g = 255; b = 0;   b += x; break;
	//	case 3: r = 0;   g = 255; b = 255; g -= x; break;
	//	case 4: r = 0;   g = 0;   b = 255; r += x; break;
	//	case 5: r = 255; g = 0;   b = 255; b -= x; break;
	//	}
	//
	//	Uint32 color = r + (g << 8) + (b << 16);
	//	Color sdl_color = *(Color*)&color;
	//	sdl_color.a = 255;
	//	return sdl_color;
	//}

	struct /*NIGHT_API*/ Color8
	{
		u8 r{ 0 };
		u8 g{ 0 };
		u8 b{ 0 };
		u8 a{ 0 };

		Color8() = default;
		Color8(u8 r, u8 g, u8 b, u8 a) : r(r), g(g), b(b), a(a) {}

		Color8(const Color& color)
		{
			r = (u8)(color.r * 255);
			g = (u8)(color.g * 255);
			b = (u8)(color.b * 255);
			a = (u8)(color.a * 255);
		}

		//static Color8 make(const Color& color)
		//{
		//	return { (u8)(color.r * 255), (u8)(color.g * 255), (u8)(color.b * 255), (u8)(color.a * 255) };
		//}
	};

}