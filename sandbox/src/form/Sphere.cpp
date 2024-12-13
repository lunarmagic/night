
#include "nightpch.h"
#include "Sphere.h"
#include "Renderer3D/Renderer3D.h"
#include "math/math.h"
#include "utility.h"

namespace night
{
	Sphere::Sphere(const SphereParams& params)
		: IForm(EFormType::Sphere)
	{
		transform(params.transform);
		_radius = params.radius;
		color(params.color);
	}

	void Sphere::on_render()
	{
		constexpr s32 segments = 32;

		for (s32 i = 0; i < segments; i++)
		{
			real t1 = (real)i / (real)(segments - 1) * R_PI * 2;
			real t2 = (real)(i + 1) / (real)(segments - 1) * R_PI * 2;

			vec4 p1;
			p1.x = cos(t1) * _radius; // TODO: handle scale
			p1.y = sin(t1) * _radius;
			p1.z = 0;
			p1.w = 1;
			p1 = transform() * p1; // TODO: don't rotate, handle linear perspective.

			vec4 p2;
			p2.x = cos(t2) * _radius;
			p2.y = sin(t2) * _radius;
			p2.z = 0;
			p2.w = 1;
			p2 = transform() * p2;

			utility::renderer().draw_line(Renderer3D::project_point_to_view_plane(p1), Renderer3D::project_point_to_view_plane(p2), color());
		}
	}
}