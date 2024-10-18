
#include "nightpch.h"
#include "Intersection.h"
#include "Box.h"
#include "math/math.h"

namespace night
{
	Intersection Intersection::intersect(ref<Box> a, ref<Box> b) // TODO: use better names.
	{
		Intersection result;

		result.a = a;
		result.b = b;

		auto planes_a = a->planes();
		auto planes_b = b->planes();

		for (s32 i = 0; i < planes_a.size(); i++)
		{
			for (s32 j = 0; j < planes_b.size(); j++)
			{
				auto& plane_a = planes_a[i];
				auto& plane_b = planes_b[j];

				// vertex #4 is ignored, only a triangle can have a normal.
				const vec3& normal_a = plane_a.normal;
				const vec3& normal_b = plane_b.normal;

				vec3 normal_i = cross(normal_a, normal_b);
				// TODO: raycast and be parallel with line, and fail.
				vec3 point_i = raycast_to_plane(plane_a.vertices[0], plane_a.vertices[1], plane_b.vertices[0], normal_b);

				auto fn = [&normal_i, &point_i](const Box::Plane& plane, const vec3& normal) -> pair<real, real>
				{
					real min_t1 = INFINITY;
					real max_t1 = -INFINITY;

					vec3 projector_x = plane.vertices[1] - plane.vertices[0];
					vec3 projector_y = cross(projector_x, normal);

					vec2 point_projected = { dot(point_i, projector_x), dot(point_i, projector_y) };
					vec2 normal_projected = { dot(point_i + normal_i, projector_x), dot(point_i + normal_i, projector_y) };

					for (s32 i = 0; i < 4; i++)
					{
						vec3 point1 = plane.vertices[i];
						vec3 point2 = plane.vertices[(i + 1) % 4];
						vec2 pp1 = { dot(point1, projector_x), dot(point1, projector_y) };
						vec2 pp2 = { dot(point2, projector_x), dot(point2, projector_y) };

						auto projection_2d = project_lines(point_projected, normal_projected, pp1, pp2);

						if (!projection_2d.is_valid() || projection_2d.b_voronoi())
						{
							continue;
						}

						min_t1 = MIN(min_t1, projection_2d.a_toi());
						max_t1 = MAX(max_t1, projection_2d.a_toi());
					}

					return { min_t1, max_t1 };
				};

				auto [min_t1, max_t1] = fn(plane_a, normal_a);
				auto [min_t2, max_t2] = fn(plane_b, normal_b);

				if (min_t1 > max_t2 || min_t2 > max_t1)
				{
					continue;
				}

				Intersection::Line line;
				line.p1 = point_i + normal_i * MAX(min_t1, min_t2);
				line.p2 = point_i + normal_i * MIN(max_t1, max_t2);
				//line.normal1 = normal_a;
				//line.normal2 = normal_b;
				line.plane1 = plane_a;
				line.plane2 = plane_b;
				result.lines.push_back(line);
			}
		}

		return result;
	}
	Intersection Intersection::intersect_test(ref<Box> a, vec3 a_pos, ref<Box> b, vec3 b_pos)
	{
		Intersection result;

		result.a = a;
		result.b = b;

		auto planes_a = a->planes();
		auto planes_b = b->planes();

		for (s32 i = 0; i < planes_a.size(); i++)
		{
			for (s32 j = 0; j < planes_b.size(); j++)
			{
				Box::Plane plane_a = planes_a[i];
				plane_a.vertices[0] += a_pos;
				plane_a.vertices[1] += a_pos;
				plane_a.vertices[2] += a_pos;
				plane_a.vertices[3] += a_pos;

				Box::Plane plane_b = planes_b[j];
				plane_b.vertices[0] += b_pos;
				plane_b.vertices[1] += b_pos;
				plane_b.vertices[2] += b_pos;
				plane_b.vertices[3] += b_pos;

				// vertex #4 is ignored, only a triangle can have a normal.
				const vec3& normal_a = plane_a.normal;
				const vec3& normal_b = plane_b.normal;

				vec3 normal_i = cross(normal_a, normal_b);
				// TODO: raycast and be parallel with line, and fail.
				vec3 point_i = raycast_to_plane(plane_a.vertices[0], plane_a.vertices[1], plane_b.vertices[0], normal_b);

				auto fn = [&normal_i, &point_i](const Box::Plane& plane, const vec3& normal) -> pair<real, real>
				{
					real min_t1 = INFINITY;
					real max_t1 = -INFINITY;

					vec3 projector_x = plane.vertices[1] - plane.vertices[0];
					vec3 projector_y = cross(projector_x, normal);

					vec2 point_projected = { dot(point_i, projector_x), dot(point_i, projector_y) };
					vec2 normal_projected = { dot(point_i + normal_i, projector_x), dot(point_i + normal_i, projector_y) };

					for (s32 i = 0; i < 4; i++)
					{
						vec3 point1 = plane.vertices[i];
						vec3 point2 = plane.vertices[(i + 1) % 4];
						vec2 pp1 = { dot(point1, projector_x), dot(point1, projector_y) };
						vec2 pp2 = { dot(point2, projector_x), dot(point2, projector_y) };

						auto projection_2d = project_lines(point_projected, normal_projected, pp1, pp2);

						if (!projection_2d.is_valid() || projection_2d.b_voronoi())
						{
							continue;
						}

						min_t1 = MIN(min_t1, projection_2d.a_toi());
						max_t1 = MAX(max_t1, projection_2d.a_toi());
					}

					return { min_t1, max_t1 };
				};

				auto [min_t1, max_t1] = fn(plane_a, normal_a);
				auto [min_t2, max_t2] = fn(plane_b, normal_b);

				if (min_t1 > max_t2 || min_t2 > max_t1)
				{
					continue;
				}

				Intersection::Line line;
				line.p1 = point_i + normal_i * MAX(min_t1, min_t2);
				line.p2 = point_i + normal_i * MIN(max_t1, max_t2);
				//line.normal1 = normal_a;
				//line.normal2 = normal_b;
				line.plane1 = plane_a;
				line.plane2 = plane_b;
				result.lines.push_back(line);
			}
		}

		return result;
	}
}