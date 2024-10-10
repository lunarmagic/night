
#include "nightpch.h"
#include "Polygon.h"
#include "math/math.h"
#include "window/IWindow.h"
#include "algorithm/algorithm.h"

namespace night
{

void Polygon::insert(const vec2& point) // TODO: just sort by winding angle.
{
	s32 left_most = -1;
	real max_d = -INFINITY;

	for (s32 k = 0; k < _points.size(); k++)
	{
		auto& p1 = _points[k];
		auto& p2 = _points[(k + 1) % _points.size()];

		real d = perp_dot(p2 - p1, point - p1); // TODO: add point even if d is positive
		if (d > max_d)
		{
			max_d = d;
			left_most = (k + 1) % _points.size(); // insert point between p1 and p2
		}
	}

	if (left_most != -1)
	{
		_points.insert(_points.begin() + left_most, point);
	}
}

real Polygon::area() const
{
	real area = 0.0;

	for (s32 i = 0; i < _points.size(); ++i)
	{
		auto& a = _points[i];
		auto& b = _points[(i + 1) % _points.size()];
		area += 0.5f * (a.x * b.y - b.x * a.y);
	}

	return abs(area);
}

real Polygon::arc_length() const
{
	return ::night::arc_length(_points, true);
}

void Polygon::draw() const
{
	if (_points.empty())
	{
		return;
	}

	constexpr real gap_size = 0.01f;
	vector<vec2> smaller = _points;
	vec2 center = { 0, 0 };
	
	for (s32 i = 0; i < smaller.size(); i++)
	{
		auto& a = smaller[i];
		center += a;
	}

	center /= smaller.size();

	for (s32 i = 0; i < smaller.size(); i++)
	{
		auto& a = smaller[i];

		a -= center;
		//a *= 1.0f - gap_size;
		a -= normalize(a) * gap_size;
		a += center;
	}

	for (s32 i = 0; i < smaller.size(); i++)
	{
		auto& a = smaller[i];
		auto& b = smaller[(i + 1) % smaller.size()];

		utility::renderer().draw_line(a, b, color);
	}
}

#define VORONOI_EPSILON 0.0001f

static u8 _a_voronoi(const LineProjectionResult& r)
{
	//LOG(r.f);
	if (r.f < 0.f)
	{
		if (r.a > VORONOI_EPSILON)     return true;
		if (r.a < r.f - VORONOI_EPSILON)       return true;
	}
	else
	{
		if (r.a < -VORONOI_EPSILON)     return true;
		if (r.a > r.f + VORONOI_EPSILON)       return true;
	}

	return false;
}

PolygonClipResult Polygon::clip(const Polygon& polygon, const vec2& plane_origin, const vec2& plane_normal)
{
	PolygonClipResult result;
	pair<s32, vec2> intersections[2];

	s32 hits = 0;

	s32 i = 0;
	for (; i < polygon._points.size(); i++)
	{
		auto& a = polygon._points[i];
		auto& b = polygon._points[(i + 1) % polygon._points.size()];

		real d = perp_dot(normalize(b - a), normalize(plane_normal)); // TODO: find a better solution to this.

		if (abs(d) < VORONOI_EPSILON)
		{
			continue;
		}

		auto intersection = project_lines(a, b, plane_origin - plane_normal, plane_origin + plane_normal);
		if (intersection.is_valid() && !_a_voronoi(intersection))
		{
			//draw_point(a + (b - a) * intersection.a_toi(), GREEN);
			intersections[0] = { i, a + (b - a) * intersection.a_toi() };
			hits++;
			break;
		}
	}

	s32 j = (s32)polygon._points.size() - 1;
	for (; j > i; j--)
	{
		auto& a = polygon._points[j];
		auto& b = polygon._points[(j + 1) % polygon._points.size()];

		real d = perp_dot(normalize(b - a), normalize(plane_normal)); // TODO: find a better solution to this.

		if (abs(d) < VORONOI_EPSILON)
		{
			continue;
		}

		auto intersection = project_lines(a, b, plane_origin - plane_normal, plane_origin + plane_normal);
		if (intersection.is_valid() && !_a_voronoi(intersection))
		{
			//draw_point(a + (b - a) * intersection.a_toi(), CYAN);
			intersections[1] = { j, a + (b - a) * intersection.a_toi() };
			hits++;
			break;
		}
	}

	if (hits < 2)
	{
		return { .result = false };
	}

	
	// first half
	for (s32 k = 0; k <= i; k++)
	{
		result.a.push_back(polygon._points[k % polygon._points.size()]);
	}

	result.a.push_back(intersections[0].second);
	result.a.push_back(intersections[1].second);

	for (s32 k = j + 1; k < polygon._points.size(); k++)
	{
		result.a.push_back(polygon._points[k % polygon._points.size()]);
	}

	// second half
	result.b.push_back(intersections[0].second);

	for (s32 k = i + 1; k <= j; k++)
	{
		result.b.push_back(polygon._points[k % polygon._points.size()]);
	}

	result.b.push_back(intersections[1].second);

	result.result = true;

	return result;
}

//void Polygon::rasterize(const function<void(const ivec2&)>& fn, real resolution)
//{
//
//}

}