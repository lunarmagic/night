#pragma once

#include "IForm.h"
#include "utility.h"
#include "color/Color.h"

namespace night
{

	struct Intersection;

	struct BoxParams
	{
		const mat4& transform{ mat4(1) };
		const vec3& extents{ 1, 1, 1 };
		const Color& color{ LIGHT };
	};

	struct Box : public IForm // TODO: add Node3D
	{
		Box(const BoxParams& params);

		//virtual vector<Line> intersect(const IForm& other) override;

		struct Plane
		{
			array<vec3, 4> vertices;
			vec3 normal;
		};

		array<Plane, 6> planes() const;
		// get the contour in global 3d space, winding order
		vector<vec4> contour();

		const array<vec4, 8>& points() const { return _points; }

	protected:

		virtual void on_render() override;

	private:

		array<vec4, 8> _points;
		Color _color{ LIGHT };
	};
}
