#pragma once

#include "core.h"
#include "INode.h"

namespace night
{

	struct NIGHT_API Node2D : public INode
	{
		const vec2& position() const;
		const vec2& scale() const;
		const vec2& rotation() const;

		void position(const vec2& position);
		void scale(const vec2& scale);
		void rotation(const vec2& rotation);

		void translate(const vec2& translation);
		void rotate(real theta);

	private:

		void update();
		vec2 _position{ vec2(0) };
		vec2 _scale{ vec2(1) };
		vec2 _rotation{ vec2(0, 1) };
	};

}