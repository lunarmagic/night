#pragma once

#include "core.h"
#include "Vertex.h"

namespace night
{

	struct AABB;

	struct QuadParams
	{
		vec2 position{ 0.0f, 0.0f }; // TODO: make quad fvec2 so we don't have to convert
		vec2 size{ 1.0f, 1.0f };
		Color color{ WHITE };
		array<vec2, 4> texture_coords
		{
			vec2{ 0.0f, 0.0f },
			vec2{ 1.0f, 0.0f },
			vec2{ 1.0f, 1.0f },
			vec2{ 0.0f, 1.0f },
		};
	};

	struct NIGHT_API Quad
	{
		Vertex vertices[6];

		Quad() = default;
		Quad(const QuadParams& params);
		Quad(const AABB& aabb, const QuadParams& params = {});
	};

}

