#pragma once

#include "color/Color.h"

namespace night
{

	struct Polygon3D
	{
		Color color{ BLUE };

		const vector<vec4>& points() const { return _points; }
		void points(const vector<vec4>& points) { _points = points; }

		void push_back(const vec4& point) { _points.push_back(point); }

		//// insert point into polygon in winding order.
		//void insert(const vec2& point);

		void pop_back() { _points.pop_back(); }

		u8 empty() { return _points.empty(); }
		void clear() { _points.clear(); }

		u32 size() const { return (u32)_points.size(); }
		//real area() const;
		//real arc_length() const;
		// TODO: add centroid function.

		void draw() const;

		vec4& operator[](s32 i) { return _points[i % _points.size()]; } // TODO: fix this
		const vec4& operator[](s32 i) const { return _points[i % _points.size()]; }

	private:

		vector<vec4> _points; // TODO: make private.
	};

}