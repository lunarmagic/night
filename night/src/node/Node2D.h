#pragma once

#include "core.h"
#include "INode.h"

namespace night
{

	struct NIGHT_API Node2D : public INode
	{
		const vec2& position() const { return _position; }
		const vec2& scale() const{ return _scale; };
		const vec2& rotation() const{ return _rotation; };

		void position(const vec2& position) { _position = position; }
		void scale(const vec2& scale) { _scale = scale; };
		void rotation(const vec2& rotation) { _rotation = rotation; };

	private:

		vec2 _position{ vec2(0) };
		vec2 _scale{ vec2(1) };
		vec2 _rotation{ vec2(0) };
	};

}