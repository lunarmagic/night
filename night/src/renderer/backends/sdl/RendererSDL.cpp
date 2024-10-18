
#include "nightpch.h"

#include <SDL.h>
#undef _main

#include "RendererSDL.h"
#include "window/backends/sdl/WindowSDL.h"
#include "log/log.h"

namespace night
{
	//RendererSDL::RendererSDL(const RendererParams& params)
	//	: IRenderer(params)
	//{
	//	WindowSDL* sdl_window = (WindowSDL*)window();
	//	_sdlRenderer = SDL_CreateRenderer(sdl_window->sdl_window(), 0, 0);

	//	if (!_sdlRenderer)
	//	{
	//		ERROR("SDL Renderer failed to initialize! SDL_Error: {0}", SDL_GetError());
	//		return;
	//	}

	//	SDL_RendererInfo info;

	//	if (SDL_GetRendererInfo(_sdlRenderer, &info) < 0)
	//	{
	//		WARNING("SDL failed to get renderer info, SDL_Error: ", SDL_GetError());
	//	}
	//}

	s32 RendererSDL::init(const RendererParams& params)
	{
		__super::init(params);
		ASSERT(window() != nullptr);

		WindowSDL* sdl_window = (WindowSDL*)window();
		_sdlRenderer = SDL_CreateRenderer(sdl_window->sdl_window(), 0, 0);

		if (!_sdlRenderer)
		{
			ERROR("SDL Renderer failed to initialize! SDL_Error: {0}", SDL_GetError());
			return -1;
		}

		//SDL_RendererInfo info;
		//
		//if (SDL_GetRendererInfo(_sdlRenderer, &info) < 0)
		//{
		//	WARNING("SDL failed to get renderer info, SDL_Error: ", SDL_GetError());
		//}

		return 1;
	}

	void RendererSDL::close()
	{
		if (_sdlRenderer)
		{
			SDL_DestroyRenderer(_sdlRenderer);
			_sdlRenderer = nullptr;
		}
	}

	void RendererSDL::update()
	{
		SDL_RenderPresent(_sdlRenderer);

		Color8 cc = Color8(clear_color());
		SDL_SetRenderDrawColor(_sdlRenderer, cc.r, cc.g, cc.b, cc.a);
		SDL_RenderClear(_sdlRenderer);
	}

	void RendererSDL::draw_point(const vec2& point, const Color& color) const
	{
		draw_line(point, { point.x, point.y - 0.025f }, color);
	}

	void RendererSDL::draw_line(const vec2& p1, const vec2& p2, const Color& color) const
	{
		WindowSDL* sdl_window = (WindowSDL*)window();
		fvec2 un1 = sdl_window->local_to_internal(p1);
		fvec2 un2 = sdl_window->local_to_internal(p2);
		set_render_draw_color(WindowSDL::sdl_color(color));
		SDL_RenderDrawLineF(_sdlRenderer, un1.x, un1.y, un2.x, un2.y);
	}

	void RendererSDL::draw_quad(const Quad& q, ref<ITexture> tx) const
	{
		ref<WindowSDL::Texture> texture;
		texture = tx;

		ASSERT(texture != nullptr);

		if (texture->sdl_texture())
		{
			WindowSDL* sdl_window = (WindowSDL*)window();
			Quad quad = q;
			quad.vertices[0].point = mvp() * quad.vertices[0].point;
			quad.vertices[1].point = mvp() * quad.vertices[1].point;
			quad.vertices[2].point = mvp() * quad.vertices[2].point;
			quad.vertices[3].point = mvp() * quad.vertices[3].point;
			quad.vertices[4].point = mvp() * quad.vertices[4].point;
			quad.vertices[5].point = mvp() * quad.vertices[5].point;

			struct _vertex
			{
				fvec2 point;
				Color8 color;
				fvec2 texture_coord;
			} vertices[6];

			vertices[0].point = sdl_window->local_to_internal((fvec2)quad.vertices[0].point);
			vertices[1].point = sdl_window->local_to_internal((fvec2)quad.vertices[1].point);
			vertices[2].point = sdl_window->local_to_internal((fvec2)quad.vertices[2].point);
			vertices[3].point = sdl_window->local_to_internal((fvec2)quad.vertices[3].point);
			vertices[4].point = sdl_window->local_to_internal((fvec2)quad.vertices[4].point);
			vertices[5].point = sdl_window->local_to_internal((fvec2)quad.vertices[5].point);

			vertices[0].color = quad.vertices[0].color;
			vertices[1].color = quad.vertices[1].color;
			vertices[2].color = quad.vertices[2].color;
			vertices[3].color = quad.vertices[3].color;
			vertices[4].color = quad.vertices[4].color;
			vertices[5].color = quad.vertices[5].color;

			vertices[0].texture_coord = quad.vertices[0].texture_coord;
			vertices[1].texture_coord = quad.vertices[1].texture_coord;
			vertices[2].texture_coord = quad.vertices[2].texture_coord;
			vertices[3].texture_coord = quad.vertices[3].texture_coord;
			vertices[4].texture_coord = quad.vertices[4].texture_coord;
			vertices[5].texture_coord = quad.vertices[5].texture_coord;

			SDL_RenderGeometry(_sdlRenderer, texture->sdl_texture(), (SDL_Vertex*)vertices, 6, nullptr, 6);
		}
		else
		{
			WARNING("Quad texture.sdl_texture() is nullptr! ");
		}
	}

	void RendererSDL::set_render_draw_color(const SDL_Color& color) const
	{
		SDL_SetRenderDrawColor(_sdlRenderer, color.r, color.g, color.b, color.a);
	}

}