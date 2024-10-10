
#include "nightpch.h"
#include "Box.h"
#include "Renderer3D/Renderer3D.h"
#include "Drawing/Line.h"
#include "math/math.h"
#include "gamemode/FormIntersectionsMode.h"
#include "algorithm/algorithm.h"

namespace night
{

	Box::Box(const BoxParams& params)
	{
		transform(params.transform);
		auto& e = params.extents;

		_points[0] = { -e.x, -e.y, -e.z, 1.0f };
		_points[1] = { e.x, -e.y, -e.z, 1.0f };
		_points[2] = { e.x, e.y, -e.z, 1.0f };
		_points[3] = { -e.x, e.y, -e.z, 1.0f };

		_points[4] = { -e.x, -e.y, e.z, 1.0f };
		_points[5] = { e.x, -e.y, e.z, 1.0f };
		_points[6] = { e.x, e.y, e.z, 1.0f };
		_points[7] = { -e.x, e.y, e.z, 1.0f };

		_color = params.color;
	}

	array<Box::Plane, 6> Box::planes() const
	{
		array<Box::Plane, 6> result;

		// front face
		result[0].vertices =
		{
			transform() * _points[3],
			transform() * _points[2],
			transform() * _points[1],
			transform() * _points[0]
		};

		result[0].normal = normalize(cross(result[0].vertices[1] - result[0].vertices[0], result[0].vertices[2] - result[0].vertices[1]));

		// back face
		result[1].vertices =
		{
			transform() * _points[4],
			transform() * _points[5],
			transform() * _points[6],
			transform() * _points[7]
		};

		result[1].normal = normalize(cross(result[1].vertices[1] - result[1].vertices[0], result[1].vertices[2] - result[1].vertices[1]));

		// side 1
		result[2].vertices =
		{
			transform() * _points[5],
			transform() * _points[4],
			transform() * _points[0],
			transform() * _points[1]
		};

		result[2].normal = normalize(cross(result[2].vertices[1] - result[2].vertices[0], result[2].vertices[2] - result[2].vertices[1]));

		// side 2
		result[3].vertices =
		{
			transform() * _points[6],
			transform() * _points[5],
			transform() * _points[1],
			transform() * _points[2]
		};

		result[3].normal = normalize(cross(result[3].vertices[1] - result[3].vertices[0], result[3].vertices[2] - result[3].vertices[1]));

		// side 3
		result[4].vertices =
		{
			transform() * _points[7],
			transform() * _points[6],
			transform() * _points[2],
			transform() * _points[3]
		};

		result[4].normal = normalize(cross(result[4].vertices[1] - result[4].vertices[0], result[4].vertices[2] - result[4].vertices[1]));

		// side 4
		result[5].vertices =
		{
			transform() * _points[4],
			transform() * _points[7],
			transform() * _points[3],
			transform() * _points[0]
		};

		result[5].normal = normalize(cross(result[5].vertices[1] - result[5].vertices[0], result[5].vertices[2] - result[5].vertices[1]));

		return result;
	}

	vector<vec4> Box::contour()
	{
		vector<vec4> result;
		array<vec2, 8> points_projected;

		for (s32 i = 0; i < _points.size(); i++)
		{
			points_projected[i] = Renderer3D::project_point_to_view_plane(transform() * _points[i]);
		}

		s32 left_most = 0;

		for (s32 i = 1; i < points_projected.size(); i++)
		{
			if (points_projected[i].x < points_projected[left_most].x)
			{
				left_most = i;
			}
		}

		s32 point_on_hull = left_most;
		s32 current;

		do
		{
			//result.push_back(points[point_on_hull]);
			result.push_back(transform() * _points[point_on_hull]);

			current = (point_on_hull + 1) % points_projected.size();
			for (s32 next = 0; next < points_projected.size(); next++)
			{
				if (orientation(points_projected[point_on_hull], points_projected[next], points_projected[current]) == EOrientation::CounterClockwise)
				{
					current = next;
				}
			}

			point_on_hull = current;
		} while (point_on_hull != left_most);

		return result;
	}

	void Box::on_render() // TODO: only debug render
	{
		auto planes_global = planes();
		
		for (s32 i = 0; i < planes_global.size(); i++)
		{
			auto& plane = planes_global[i];
			vec2 proj[4];
			for (s32 j = 0; j < plane.vertices.size(); j++)
			{
				proj[j] = Renderer3D::project_point_to_view_plane(vec4(plane.vertices[j].x, plane.vertices[j].y, plane.vertices[j].z, 1.0f));
			}
		
			for (s32 j = 0; j < 4; j++)
			{
				utility::renderer().draw_line(proj[j], proj[(j + 1) % 4], _color);
			}
		
		
			//vec2 p1 = Renderer3D::project_point_to_view_plane(vec4(plane.vertices[0], 1.0f));
			//vec2 p2 = Renderer3D::project_point_to_view_plane(vec4(plane.vertices[0] + plane.normal, 1.0f));
			//Window::get().draw_line(p1, p2, GREEN);
		}

		//vec2 points_proj[8];
		//for (s32 i = 0; i < 8; i++)
		//{
		//	points_proj[i] = Renderer3D::project_point_to_view_plane((transform() * _points[i]));
		//}

		//// front face
		//Window::get().draw_line(points_proj[0], points_proj[1], _color);
		//Window::get().draw_line(points_proj[1], points_proj[2], _color);
		//Window::get().draw_line(points_proj[2], points_proj[3], _color);
		//Window::get().draw_line(points_proj[3], points_proj[0], _color);

		//// back face
		//Window::get().draw_line(points_proj[4], points_proj[5], _color);
		//Window::get().draw_line(points_proj[5], points_proj[6], _color);
		//Window::get().draw_line(points_proj[6], points_proj[7], _color);
		//Window::get().draw_line(points_proj[7], points_proj[4], _color);

		//// side faces
		//Window::get().draw_line(points_proj[0], points_proj[4], _color);
		//Window::get().draw_line(points_proj[1], points_proj[5], _color);
		//Window::get().draw_line(points_proj[2], points_proj[6], _color);
		//Window::get().draw_line(points_proj[3], points_proj[7], _color);
	}

}