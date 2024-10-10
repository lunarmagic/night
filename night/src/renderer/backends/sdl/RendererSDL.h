#pragma once

#include "core.h"
#include "renderer/IRenderer.h"

struct SDL_Renderer;

namespace night
{

	struct NIGHT_API RendererSDL : public IRenderer
	{
		//RendererSDL(const RendererParams& params);
		virtual s32 init(const RendererParams& params) override;
		virtual void update() override;
		virtual void close() override;
		virtual void draw_point(const vec2& point, const Color& color) const override;
		virtual void draw_line(const vec2& p1, const vec2& p2, const Color& color) const override;
		virtual void draw_quad(const Quad& quad, ref<ITexture> texture) const override;

		void set_render_draw_color(const SDL_Color& color) const;

		SDL_Renderer* sdl_renderer() const { return _sdlRenderer; }

	private:

		SDL_Renderer* _sdlRenderer;
	};

}