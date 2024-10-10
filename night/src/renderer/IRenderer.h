#pragma once

#include "core.h"
#include "color/Color.h"
#include "texture/ITexture.h"
#include "geometry/Quad.h"

namespace night
{
	struct IWindow;

	struct /*NIGHT_API*/ RendererParams
	{
		IWindow* window;
	};

	struct NIGHT_API IRenderer
	{
		//IRenderer(const RendererParams& params);
		virtual s32 init(const RendererParams& params);
		virtual void update() = 0;
		virtual void close();
		virtual void draw_point(const vec2& point, const Color& color) const = 0;
		virtual void draw_line(const vec2& p1, const vec2& p2, const Color& color) const = 0;
		virtual void draw_quad(const Quad& quad, ref<ITexture> texture) const = 0;

	protected:

		IWindow* window() const { return _window; }

	private:

		IWindow* _window;
		// TODO: mvp
	};

}