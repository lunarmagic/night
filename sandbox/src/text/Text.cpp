
#include "nightpch.h"
#include "Text.h"
#include "geometry/Quad.h"
#include "utility.h"

namespace night
{

	Text::Text(const TextParams& params)
	{
		_text = params.text;
		//_font = params.font;
		_color = params.color;
		position(params.position);
		scale(params.scale);

		_font = utility::window().find_texture(params.font);
		if (_font == nullptr)
		{
			WARNING(name(), "'s font is not found in window!");
			destroy();
		}
	}

	void Text::on_render()
	{
		const vec2& p = position();
		const vec2& s = scale();

		vec2 point = /*p + */s / 2.0; // TODO: add alignment

		for (s32 i = 0; i < _text.size(); i++)
		{
			auto& chr = _text[i];

			ivec2 ascii_index;
			ascii_index.x = chr % 16;
			ascii_index.y = chr / 16;

			array<vec2, 4> texture_coords;
			texture_coords[0] = vec2(real(ascii_index.x) / 16.0f, real(ascii_index.y) / 16.0f);
			texture_coords[1] = vec2(real(ascii_index.x + 1) / 16.0f, real(ascii_index.y) / 16.0f);
			texture_coords[2] = vec2(real(ascii_index.x + 1) / 16.0f, real(ascii_index.y + 1) / 16.0f);
			texture_coords[3] = vec2(real(ascii_index.x) / 16.0f, real(ascii_index.y + 1) / 16.0f);

			Quad quad(
				{
					.position = vec2(point.x + (real)i * (s.x * 2), point.y),
					.size = s,
					.color = _color,
					.texture_coords = texture_coords
				}
			);

			utility::renderer().draw_quad(quad, _font);
		}
	}

}