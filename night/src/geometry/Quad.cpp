
#include "nightpch.h"
#include "Quad.h"
#include <SDL.h>
//#include "window/IWindow.h"
//#include "window/backends/sdl/WindowSDL.h"
#include "aabb/aabb.h"

namespace night
{

Quad::Quad(const QuadParams& params)
{
	vertices[0].point = vec4(vec2(-params.size.x, -params.size.y) + params.position, 0.0f, 1.0f);
	vertices[1].point = vec4(vec2(params.size.x, -params.size.y) + params.position, 0.0f, 1.0f);
	vertices[2].point = vec4(vec2(params.size.x, params.size.y) + params.position, 0.0f, 1.0f);

	vertices[3].point = vec4(vec2(params.size.x, params.size.y) + params.position, 0.0f, 1.0f);
	vertices[4].point = vec4(vec2(-params.size.x, params.size.y) + params.position, 0.0f, 1.0f);
	vertices[5].point = vec4(vec2(-params.size.x, -params.size.y) + params.position, 0.0f, 1.0f);

	//vertices[1].point = vec4({ params.size.x, -params.size.y } + params.position;
	//vertices[2].point = vec4({ params.size.x, params.size.y } + params.position;
	//					   4(
	//vertices[3].point = vec4({ params.size.x, params.size.y } + params.position;
	//vertices[4].point = vec4({ -params.size.x,  params.size.y } + params.position;
	//vertices[5].point = vec4({ -params.size.x, -params.size.y } + params.position;

	//vertices[0].point.z = 0.0f;
	//vertices[1].point.z = 0.0f;
	//vertices[2].point.z = 0.0f;
	//vertices[3].point.z = 0.0f;
	//vertices[4].point.z = 0.0f;
	//vertices[5].point.z = 0.0f;

	//vertices[0].point.w = 1.0f;
	//vertices[1].point.w = 1.0f;
	//vertices[2].point.w = 1.0f;
	//vertices[3].point.w = 1.0f;
	//vertices[4].point.w = 1.0f;
	//vertices[5].point.w = 1.0f;

	Color8 color8(params.color);

	vertices[0].color = color8;
	vertices[1].color = color8;
	vertices[2].color = color8;
	vertices[3].color = color8;
	vertices[4].color = color8;
	vertices[5].color = color8;

	vertices[0].texture_coord = params.texture_coords[0];
	vertices[1].texture_coord = params.texture_coords[1];
	vertices[2].texture_coord = params.texture_coords[2];

	vertices[3].texture_coord = params.texture_coords[2];
	vertices[4].texture_coord = params.texture_coords[3];
	vertices[5].texture_coord = params.texture_coords[0];
}

Quad::Quad(const AABB& aabb, const QuadParams& params)
{
	//vertices[0].point = vec2{ aabb.left, aabb.top };
	//vertices[1].point = vec2{ aabb.right, aabb.top };
	//vertices[2].point = vec2{ aabb.right, aabb.bottom };

	//vertices[3].point = vec2{ aabb.right, aabb.bottom };
	//vertices[4].point = vec2{ aabb.left,  aabb.bottom };
	//vertices[5].point = vec2{ aabb.left, aabb.top };

	//vertices[0].point.z = 0.0f;
	//vertices[1].point.z = 0.0f;
	//vertices[2].point.z = 0.0f;
	//vertices[3].point.z = 0.0f;
	//vertices[4].point.z = 0.0f;
	//vertices[5].point.z = 0.0f;

	//vertices[0].point.w = 1.0f;
	//vertices[1].point.w = 1.0f;
	//vertices[2].point.w = 1.0f;
	//vertices[3].point.w = 1.0f;
	//vertices[4].point.w = 1.0f;
	//vertices[5].point.w = 1.0f;

	vertices[0].point = vec4(vec2(aabb.left, aabb.top), 0.0f, 1.0f);
	vertices[1].point = vec4(vec2(aabb.right, aabb.top), 0.0f, 1.0f);
	vertices[2].point = vec4(vec2(aabb.right, aabb.bottom), 0.0f, 1.0f);

	vertices[3].point = vec4(vec2(aabb.right, aabb.bottom), 0.0f, 1.0f);
	vertices[4].point = vec4(vec2(aabb.left, aabb.bottom), 0.0f, 1.0f);
	vertices[5].point = vec4(vec2(aabb.left, aabb.top), 0.0f, 1.0f);

	Color8 color8(params.color);

	vertices[0].color = color8;
	vertices[1].color = color8;
	vertices[2].color = color8;
	vertices[3].color = color8;
	vertices[4].color = color8;
	vertices[5].color = color8;

	vertices[0].texture_coord = params.texture_coords[0];
	vertices[1].texture_coord = params.texture_coords[1];
	vertices[2].texture_coord = params.texture_coords[2];

	vertices[3].texture_coord = params.texture_coords[2];
	vertices[4].texture_coord = params.texture_coords[3];
	vertices[5].texture_coord = params.texture_coords[0];
};

}