
#include "nightpch.h"
#include "gjk.h"
#include "math/math.h"

#define GJK_SWEEP_MAX_ITERATIONS 32

namespace night
{
	vec2 gjk::support(const vec2* vertices, s32 count, const vec2& direction)
	{
		vec2 result = {};
		real max_dot = -INFINITY;

		for (s32 i = 0; i < count; i++)
		{
			real new_d = dot(direction, vertices[i]);
			if (new_d > max_dot)
			{
				max_dot = new_d;
				result = vertices[i];
			};
		}

		return result;
	}

	u8 gjk::overlap(const vec2* vertices_a, s32 count_a, const vec2* vertices_b, s32 count_b)
	{
		// get support of minkowski difference.
		auto support_m = [&](const vec2& direction) -> vec2
		{
			vec2 supa = support(vertices_a, count_a, direction);
			vec2 supb = support(vertices_b, count_b, -direction);
			return supa - supb; // TODO: fix negative scale!
		};

		Simplex simplex;
		vec2 direction = { -1.0f, 0.0f };
		vec2 opposite_direction = -direction;
		simplex.make_line(support_m(direction), support_m(opposite_direction));

		constexpr vec2 origin = { 0, 0 };

		for (s32 i = 0; i < GJK_SWEEP_MAX_ITERATIONS; i++)
		{
			if (simplex.is_line())
			{
				// get direction perpendicular to the axis of a and b.
				const vec2& a = simplex.vertices[0];
				const vec2& b = simplex.vertices[1];

				vec2 ab = b - a;
				vec2 ao = origin - a;

				direction = normalize(triple_cross(ab, ao, ab));

				vec2 s = support_m(direction);

				if (s == a || s == b)
				{
					//vec2 s2 = support_m(-direction); // TODO: fix very stupid solution
					//simplex.push_vertex(s2);
					return false;
				}

				simplex.push_vertex(s);
			}
			else // triangle case
			{
				const vec2& c = simplex.vertices[0];
				const vec2& b = simplex.vertices[1];
				const vec2& a = simplex.vertices[2];

				const vec2 ab = b - a;
				const vec2 ac = c - a;
				const vec2 ao = origin - a;

				const vec2 ab_perp = triple_cross(ac, ab, ab);
				const vec2 ac_perp = triple_cross(ab, ac, ac);

				const real dot_ab = dot(ab_perp, ao);
				const real dot_ac = dot(ac_perp, ao);

				if (dot_ab > dot_ac && dot_ab > 0.0f)
				{
					simplex.make_line(b, a);
					continue;
				}
				else if (dot_ac > 0.0f)
				{
					simplex.make_line(c, a);
					continue;
				}

				return true;
			}
		}

		WARNING("Failed to find simplex in max gjk iterations!");
		return false;
	}
}