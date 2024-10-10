
#include "nightpch.h"

#include <SDL.h>
#undef _main

#include <SDL_image.h>

#include "WindowSDL.h"
#include "application/Application.h"
#include "log/log.h"
#include "geometry/Quad.h"
#include "color/Color.h"
#include "event/MouseEvent.h"
#include "event/ApplicationEvent.h"
#include "event/KeyEvent.h"
#include "renderer/IRenderer.h"
#include "renderer/backends/sdl/RendererSDL.h"
#include <stdio.h>
#include <iostream>

namespace night
{

	//const Application::WindowParams& WindowSDL::helper_function(const Application::WindowParams& params)
	//{
	//	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	//	{
	//		ERROR("SDL failed to initialize! SDL_Error: {0}", SDL_GetError());
	//		return params;
	//	}

	//	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	//	_sdlWindow = SDL_CreateWindow(params.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, params.width, params.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE); // TODO: add flags to params
	//	if (_sdlWindow == NULL)
	//	{
	//		ERROR("SDL Window failed to initialize! SDL_Error: {0}", SDL_GetError());
	//		return params;
	//	}

	//	return params;
	//}

	s32 WindowSDL::init(const Application::WindowParams& params)
		//: IWindow(helper_function(params))
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			ERROR("SDL failed to initialize! SDL_Error: {0}", SDL_GetError());
			return -1;
		}

		//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

		_sdlWindow = SDL_CreateWindow(params.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, params.width, params.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE); // TODO: add flags to params
		if (_sdlWindow == NULL)
		{
			ERROR("SDL Window failed to initialize! SDL_Error: {0}", SDL_GetError());
			return -1;
		}

		if (__super::init(params) < 1)
		{
			return -1;
		}

		TRACE("Window Initialized.");

		s32 img_flags = IMG_INIT_PNG;
		if (!(IMG_Init(img_flags) & img_flags))
		{
			ERROR("SDL_image could not initialize! SDL_image Error: ", IMG_GetError());
		}

		_frameTick = SDL_GetTicks();

		return 1;
	}

	//WindowSDL::~WindowSDL()
	void WindowSDL::close()
	{
		__super::close();
		SDL_DestroyWindow(_sdlWindow);
		SDL_Quit();
	}

	void WindowSDL::update() // TODO: map sdl events to our own events
	{
		auto& callback = event_callback();

		if (callback)
		{
			SDL_Event event;

			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
				case SDL_QUIT:
				{
					WindowCloseEvent e;
					callback(e);
					break;
				}

				case SDL_KEYDOWN: // TODO: map sdl keycodes to our own keycodes
				{
					KeyPressedEvent e(event.key.keysym.scancode, event.key.repeat != 0);
					callback(e);
					break;
				}

				case SDL_KEYUP:
				{
					KeyReleasedEvent e(event.key.keysym.scancode);
					callback(e);
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					MouseButtonPressedEvent e(event.button.button);
					callback(e);
					break;
				}

				case SDL_MOUSEBUTTONUP:
				{
					MouseButtonReleasedEvent e(event.button.button);
					callback(e);
					break;
				}

				case SDL_MOUSEWHEEL:
				{
					MouseWheelEvent e((real)event.wheel.x, (real)event.wheel.y);
					callback(e);
					break;
				}

				case SDL_MOUSEMOTION:
				{
					MouseMotionEvent e((real)event.motion.xrel, (real)event.motion.yrel);
					callback(e);
					break;
				}

				case SDL_WINDOWEVENT:
				{
					switch (event.window.event)
					{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					{
						WindowResizeEvent e(event.window.data1, event.window.data2);
						callback(e);
						on_resize(e); // TODO: may want to call this from application.
						break;
					}
					}
				}
				}
			}
		}

		//SDL_RenderPresent(_renderer);

		//SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
		//SDL_RenderClear(_renderer);

		renderer().update();

		{ // TODO: impl this better
			u32 delta_ticks = SDL_GetTicks() - _frameTick;
			if (u32(1000.0f / fps()) > delta_ticks)
				SDL_Delay(u32(1000.0 / fps()) - delta_ticks);

			delta_time((real)(SDL_GetTicks() - _frameTick) / 1000.0f);
		}

		_frameTick = SDL_GetTicks();
		//_timeElapsed += _deltaTime;
		time_elapsed(time_elapsed() + delta_time());
	}

	//void WindowSDL::render()
	//{

	//}

	//void WindowSDL::draw_point(const vec2& point, const Color& color) const
	//{
	//	draw_line(point, { point.x, point.y - 0.025f }, color);
	//}

	//void WindowSDL::draw_line(const vec2& p1, const vec2& p2, const Color& color) const
	//{
	//	fvec2 un1 = local_to_internal(p1);
	//	fvec2 un2 = local_to_internal(p2);
	//	set_render_draw_color(sdl_color(color));
	//	SDL_RenderDrawLineF(_renderer, un1.x, un1.y, un2.x, un2.y);
	//}

	//void WindowSDL::draw_quad(const Quad& q, ref<ITexture> tx) const
	//{
	//	ref<WindowSDL::Texture> texture;
	//	texture = tx;

	//	ASSERT(texture != nullptr);

	//	if (texture->sdl_texture())
	//	{
	//		//struct
	//		//{
	//		//	fvec2 point;
	//		//	Color8 color;
	//		//	fvec2 texture_coord;
	//		//} vertices[6];
	//		Quad quad = q;

	//		quad.vertices[0].point = local_to_internal(q.vertices[0].point);
	//		quad.vertices[1].point = local_to_internal(q.vertices[1].point);
	//		quad.vertices[2].point = local_to_internal(q.vertices[2].point);
	//		quad.vertices[3].point = local_to_internal(q.vertices[3].point);
	//		quad.vertices[4].point = local_to_internal(q.vertices[4].point);
	//		quad.vertices[5].point = local_to_internal(q.vertices[5].point);

	//		//quad.vertices[0].color = quad.vertices[0].color;
	//		//quad.vertices[1].color = quad.vertices[1].color;
	//		//quad.vertices[2].color = quad.vertices[2].color;
	//		//quad.vertices[3].color = quad.vertices[3].color;
	//		//quad.vertices[4].color = quad.vertices[4].color;
	//		//quad.vertices[5].color = quad.vertices[5].color;

	//		//quad.vertices[0].texture_coord = (fvec2)quad.vertices[0].texture_coord;
	//		//quad.vertices[1].texture_coord = (fvec2)quad.vertices[1].texture_coord;
	//		//quad.vertices[2].texture_coord = (fvec2)quad.vertices[2].texture_coord;
	//		//quad.vertices[3].texture_coord = (fvec2)quad.vertices[3].texture_coord;
	//		//quad.vertices[4].texture_coord = (fvec2)quad.vertices[4].texture_coord;
	//		//quad.vertices[5].texture_coord = (fvec2)quad.vertices[5].texture_coord;

	//		SDL_RenderGeometry(_renderer, texture->sdl_texture(), (SDL_Vertex*)quad.vertices, 6, nullptr, 6);
	//	}
	//	else
	//	{
	//		WARNING("Quad texture.sdl_texture() is nullptr! ");
	//	}
	//}

	ref<ITexture> WindowSDL::create_texture(const string& id, const TextureParams& params)
	{
		auto i = find_texture(id);
		if (i != nullptr)
		{
			WARNING("Texture already created!, id: ", id);
			return i;
		}

		sref<WindowSDL::Texture> texture(new WindowSDL::Texture(this, params));
		textures()[id] = (sref<ITexture>)texture;

		TRACE("Window created texture, id: ", id);

		return (ref<ITexture>)texture;
	}

	vec2 WindowSDL::mouse() const
	{
		s32 x;
		s32 y;
		SDL_GetMouseState(&x, &y);

		return internal_to_local({ x, y });
	}

	pair<IRenderer*, RendererParams> WindowSDL::create_renderer()
	{
		return { new RendererSDL, { .window = this } };
	}

	ref<ISurface> WindowSDL::create_surface(const string& id, const SurfaceParams& params)
	{
		auto i = find_surface(id);
		if (i != nullptr)
		{
			WARNING("Texture already created!, id: ", id);
			return i;
		}

		sref<WindowSDL::Surface> surface(new WindowSDL::Surface(params));
		surfaces()[id] = (sref<ISurface>)surface;

		TRACE("Window created texture, id: ", id);

		return (ref<ISurface>)surface;
	}

	SDL_Color WindowSDL::sdl_color(const Color& color) 
	{
		return { (u8)(color.r * 255), (u8)(color.g * 255), (u8)(color.b * 255), (u8)(color.a * 255) };
	}

	//void WindowSDL::set_render_draw_color(const SDL_Color& color) const
	//{
	//	SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
	//}

	fvec2 WindowSDL::local_to_internal(const vec2& coordinate) const // TODO: handle aspect ratio
	{
		s32 w = width();
		s32 h = height();

		fvec2 result = { (r32)coordinate.x, (r32)coordinate.y };
		result.x *= h < w ? (r32)h / (r32)w : 1.0f;
		result.y *= w < h ? (r32)w / (r32)h : 1.0f;
		result.x += 1.0f;
		result.y += 1.0f;
		result.x *= w / 2;
		result.y *= h / 2;
		return result;
	}

	fvec2 WindowSDL::local_to_internal(const fvec2& coordinate) const
	{
		s32 w = width();
		s32 h = height();

		fvec2 result = { coordinate.x, coordinate.y };
		result.x *= h < w ? (r32)h / (r32)w : 1.0f;
		result.y *= w < h ? (r32)w / (r32)h : 1.0f;
		result.x += 1.0f;
		result.y += 1.0f;
		result.x *= w / 2;
		result.y *= h / 2;
		return result;
	}

	vec2 WindowSDL::internal_to_local(const ivec2& coordinate) const
	{
		s32 w = width();
		s32 h = height();

		vec2 result = { (real)coordinate.x, (real)coordinate.y };
		result.x /= (real)w / 2;
		result.y /= (real)h / 2;
		result.x -= 1.0f;
		result.y -= 1.0f;
		result.x /= h < w ? (real)h / (real)w : 1.0f;
		result.y /= w < h ? (real)w / (real)h : 1.0f;
		return result;
	}

	WindowSDL::Texture::Texture(WindowSDL* const window, const TextureParams& params)
		//: ITexture(params)
	{
		ASSERT(window != nullptr);

		_sdlWindow = window;

		if (!params.path.empty())
		{
			SDL_Surface* sdl_surface = IMG_Load(params.path.c_str());

			if (!sdl_surface)
			{
				WARNING("SDL_Image failed to load image, path: ", params.path);
			}

			_sdlTexture = SDL_CreateTextureFromSurface(((RendererSDL&)window->renderer()).sdl_renderer(), sdl_surface);

			if (!_sdlTexture)
			{
				WARNING("SDL failed to create texture from surface!, SDL_error: ", SDL_GetError());
				return;
			}
		}
		else if (params.surface != nullptr) // TODO: add error message
		{
			ref<WindowSDL::Surface> surface;
			surface = params.surface;

			_sdlTexture = SDL_CreateTextureFromSurface(((RendererSDL&)window->renderer()).sdl_renderer(), surface->sdl_surface());

			if (!_sdlTexture)
			{
				WARNING("SDL failed to create texture from surface!, SDL_error: ", SDL_GetError());
				return;
			}
		}
		else
		{
			_sdlTexture = SDL_CreateTexture(((RendererSDL&)window->renderer()).sdl_renderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, params.width, params.height);

			if (!_sdlTexture)
			{
				WARNING("SDL failed to create texture!, SDL_error: ", SDL_GetError());
				return;
			}
		}

		//s32 w, h;

		TextureParams new_params = {};
		
		s32 error = SDL_QueryTexture(_sdlTexture, 0, 0, &new_params.width, &new_params.height);
		if (error < 0)
		{
			ERROR("SDL_QueryTexture failed!, SDL_Error: ", SDL_GetError());
		}

		ITexture::ITexture(new_params);

		//width(w);
		//height(h);
	}

	WindowSDL::Texture::~Texture()
	{
		if (_sdlTexture)
		{
			SDL_DestroyTexture(_sdlTexture);
		}
	}

	WindowSDL::Surface::Surface(const SurfaceParams& params)
		: ISurface(params)
	{
		if (params.path.empty())
		{
			// create blank surface
			_sdlSurface = SDL_CreateRGBSurface(
				0,
				width(),
				height(),
				32,
				0x000000FF,
				0x0000FF00,
				0x00FF0000,
				0xFF000000
			);

			if (!_sdlSurface)
			{
				WARNING("SDL failed to create surface! SDL_Error: ", SDL_GetError());
			}
		}
		else
		{
			_sdlSurface = IMG_Load(params.path.c_str());

			if (!_sdlSurface)
			{
				WARNING("SDL_Image failed to load image, path: ", params.path);
			}
		}
	}

	WindowSDL::Surface::~Surface()
	{
		if (_sdlSurface)
		{
			SDL_FreeSurface(_sdlSurface);
		}
	}

	Color8* WindowSDL::Surface::pixels() const
	{
		if (_sdlSurface)
		{
			return (Color8*)_sdlSurface->pixels;
		}

		WARNING("SDL surface is nullptr!");
		return nullptr;
	}

}