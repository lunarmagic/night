#pragma once

#include "core.h"
#include "INode.h"

namespace night
{

	struct NIGHT_API Node3D : public INode
	{
		Node3D();

		void translate(const vec3& translation);
		void scale(const vec3& scale);
		void rotate(const vec3& axis, real radians);

		void transform(const mat4& transform) { _transform = transform; }
		const mat4& transform() const { return _transform; }

	private:

		mat4 _transform;
	};

}