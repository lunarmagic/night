#pragma once

#include "node/Node2D.h"
#include "window/IWindow.h"
#include "color/Color.h"

namespace night
{

	struct TextParams
	{
		const string& text{ "" };
		const string& font{ "Font10x" };
		const vec2& position{ vec2(0) }; // TODO: transform2D
		const vec2& scale{ vec2(1) };
		const Color& color{ LIGHT };
	};

	struct Text : public Node2D
	{
		Text(const TextParams& params);

		void text(const string& text) { _text = text; }
		void color(const Color& color) { _color = color; }

	protected:

		virtual void on_render() override;

	private:

		string _text;
		ref<ITexture> _font;
		Color _color;
		// TODO: alignment
	};

}