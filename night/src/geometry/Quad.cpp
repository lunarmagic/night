
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
	vertices[0].point = vec2{ -params.size.x, -params.size.y } + params.position;
	vertices[1].point = vec2{ params.size.x, -params.size.y } + params.position;
	vertices[2].point = vec2{ params.size.x, params.size.y } + params.position;

	vertices[3].point = vec2{ params.size.x, params.size.y } + params.position;
	vertices[4].point = vec2{ -params.size.x,  params.size.y } + params.position;
	vertices[5].point = vec2{ -params.size.x, -params.size.y } + params.position;

	//SDL_Color color = WindowSDL::sdl_color(params.color); // TODO: add Color8::make(Color& color)
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
	vertices[0].point = vec2{ aabb.left, aabb.top };
	vertices[1].point = vec2{ aabb.right, aabb.top };
	vertices[2].point = vec2{ aabb.right, aabb.bottom };

	vertices[3].point = vec2{ aabb.right, aabb.bottom };
	vertices[4].point = vec2{ aabb.left,  aabb.bottom };
	vertices[5].point = vec2{ aabb.left, aabb.top };

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