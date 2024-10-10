#pragma once

//#include "utility.h"
#include "aabb/aabb.h"

namespace night // TODO: put everything in this namespace
{

	struct Line;

	enum class EOrientation
	{
		Colinear = 0,
		Clockwise = 1,
		CounterClockwise = 2
	};

	//real polygon_area(const vector<vec2>& polygon);

	EOrientation orientation(const vec2& p, const vec2& q, const vec2& r);

	vector<vec2> make_convex_hull(const vector<vec2>& points);

	vector<vec2> approximate_convex_to_poly_count(const vector<vec2>& points, s32 poly_count);

	real arc_length(const vector<vec2>& arc, u8 closed);

	real perpendicular_distance(const vec2& lineStart, const vec2& lineEnd, const vec2& pt); // TODO: implement this.

	vector<vec2> ramer_douglas_peucker(const vector<vec2>& points, real epsilon); // TODO: figure out why there is extra points added.

	//struct PolarCoordinate
	//{
	//	union
	//	{
	//		struct
	//		{
	//			s32 radius;
	//			s32 theta;
	//		};
	//
	//		u64 hash;
	//	};
	//
	//	u8 operator==(const PolarCoordinate& rhs) const
	//	{
	//		return radius == rhs.radius && theta == rhs.theta;
	//	}
	//};
	//
	//namespace std
	//{
	//	template<>
	//	struct hash<PolarCoordinate>
	//	{
	//		uint64_t operator()(const PolarCoordinate& key) const
	//		{
	//			return hash<uint64_t>()((uint64_t)key.hash);
	//		}
	//	};
	//};
	//
	//struct LineDetectionAccumulator
	//{
	//	//umap<s32, s32> accumulator;
	//
	//	umap<PolarCoordinate, s32> accumulator;
	//	s32 width;
	//	s32 height;
	//
	//	//real img_width;
	//	//real img_height;
	//
	//	AABB aabb;
	//
	//	vector<Line> get_lines(s32 threshhold);
	//};
	//
	//LineDetectionAccumulator detect_lines(const vector<vec2>& points, const vec2& origin = { 0.0f, 0.0f });

}