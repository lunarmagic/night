#pragma once

#include "core.h"
#include "utility.h"
#include "application/Application.h"
#include "window/IWindow.h"

#include "color/Color.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Color;

namespace night
{

struct Event;
struct Color;
struct Quad;
struct WindowResizeEvent;

struct NIGHT_API WindowSDL : public IWindow
{
	virtual s32 init(const Application::WindowParams& params) override;
	//virtual ~WindowSDL();
	virtual void close() override;

	virtual void update() override;
	//virtual void render() override;

	//virtual void draw_point(const vec2& point, const Color& color) const override;
	//virtual void draw_line(const vec2& p1, const vec2& p2, const Color& color) const override;
	//virtual void draw_quad(const Quad& quad, ref<ITexture> texture) const override;

	struct Surface : public ISurface
	{
		Surface(const SurfaceParams& params);
		~Surface();

		virtual Color8* pixels() const override;
		SDL_Surface* sdl_surface() const { return _sdlSurface; }

	private:

		SDL_Surface* _sdlSurface{ nullptr };
	};

	struct Texture : public ITexture
	{
		Texture() = default;
		Texture(WindowSDL* const window, const TextureParams& params);
		~Texture();

		SDL_Texture* sdl_texture() const { return _sdlTexture; }

	private:

		WindowSDL* _sdlWindow{ nullptr };
		SDL_Texture* _sdlTexture{ nullptr };
	};

	virtual ref<ITexture> create_texture(const string& id, const TextureParams& params) override;
	virtual ref<ISurface> create_surface(const string& id, const SurfaceParams& params) override;

	//SDL_Renderer* renderer() const { return _renderer; }; // TODO: remove

	static SDL_Color sdl_color(const Color& color);

	virtual vec2 mouse() const override;

	SDL_Window* sdl_window() const { return _sdlWindow; }

	fvec2 local_to_internal(const vec2& coordinate) const;
	fvec2 local_to_internal(const fvec2& coordinate) const;
	vec2 internal_to_local(const ivec2& coordinate) const;

protected:

	virtual pair<IRenderer*, RendererParams> create_renderer() override;

private:

	SDL_Window* _sdlWindow{ nullptr };
	//SDL_Renderer* _renderer{ nullptr };

	u32 _frameTick;

	//const Application::WindowParams& helper_function(const Application::WindowParams& params);

	//void set_render_draw_color(const SDL_Color& color) const;
};

}