#pragma once

#include "utility.h"
#include "color/Color.h"

namespace night
{

struct PolygonClipResult;

struct Polygon
{
	Color color{ BLUE };

	const vector<vec2>& points() const { return _points; }
	void points(const vector<vec2>& points) { _points = points; }

	void push_back(const vec2& point) { _points.push_back(point); }

	// insert point into polygon in winding order.
	void insert(const vec2& point);

	void pop_back() { _points.pop_back(); }

	u8 empty() { return _points.empty(); }
	void clear() { _points.clear(); }

	u32 size() const { return (u32)_points.size(); }
	real area() const;
	real arc_length() const;
	// TODO: add centroid function.

	void draw() const;

	vec2& operator[](s32 i) { return _points[i % _points.size()]; } // TODO: fix this
	const vec2& operator[](s32 i) const { return _points[i % _points.size()]; }

	// TODO: add convex function
	static PolygonClipResult clip(const Polygon& polygon, const vec2& plane_origin, const vec2& plane_normal);
	//static void rasterize(const function<void(const ivec2&)>& fn, real resolution = 1.0f);

private:

	vector<vec2> _points;
};

struct PolygonClipResult
{
	u8 result{ false };
	Polygon a;
	Polygon b;
};

}