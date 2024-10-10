
#include "nightpch.h"
#include "algorithm.h"
#include "log/log.h"
#include "math/math.h"
#include "Drawing/Line.h"

namespace night
{

	EOrientation orientation(const vec2& a, const vec2& b, const vec2& c) // TODO: fix this
	{
		real o = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);

		if (o < 0.0f)
		{
			return EOrientation::CounterClockwise;
		}
		else if (o > 0.0f)
		{
			return EOrientation::Clockwise;
		}
		else
		{
			return EOrientation::Colinear;
		}
	}

	vector<vec2> make_convex_hull(const vector<vec2>& points) // TODO: fix crash
	{
		if (points.size() < 3)
		{
			WARNING("Not enough points to make convex hull.");
			return {};
		}

		vector<vec2> result;

		s32 left_most = 0;

		for (s32 i = 1; i < points.size(); i++)
		{
			if (points[i].x < points[left_most].x)
			{
				left_most = i;
			}
		}

		s32 point_on_hull = left_most;
		s32 current;

		do
		{
			result.push_back(points[point_on_hull]);

			current = (point_on_hull + 1) % points.size();
			for (s32 next = 0; next < points.size(); next++)
			{
				if (orientation(points[point_on_hull], points[next], points[current]) == EOrientation::CounterClockwise)
				{
					current = next;
				}
			}

			point_on_hull = current;
		} while (point_on_hull != left_most);

		return result;
	}

	vector<vec2> approximate_convex_to_poly_count(const vector<vec2>& points, s32 poly_count)
	{
		if (points.size() < poly_count)
		{
			return points;
		}

		vector<vec2> result = points;
		pair<real, s32> min_dist;

		while (result.size() > poly_count)
		{
			min_dist = { INFINITY, -1 };

			for (s32 i = 0; i < result.size(); i++)
			{
				const vec2& line_a = result[i];
				const vec2& line_b = result[(i + 2) % result.size()];
				const vec2& point = result[(i + 1) % result.size()];
				real dist = perpendicular_distance(line_a, line_b, point);

				if (dist < min_dist.first)
				{
					min_dist = { dist, (i + 1) % (s32)result.size() };
				}
			}

			result.erase(result.begin() + min_dist.second);

		}

		return result;
	}

	real arc_length(const vector<vec2>& arc, u8 closed)
	{
		real result = 0.0;

		for (s32 i = 0; i < arc.size() - 1; i++)
		{
			auto& p1 = arc[i];
			auto& p2 = arc[i + 1];

			result += distance(p1, p2);
		}

		if (closed)
		{
			result += distance(arc.front(), arc.back());
		}

		return result;
	}

	real perpendicular_distance(const vec2& lineStart, const vec2& lineEnd, const vec2& pt) // TODO: implement this.
	{
		double dx = lineEnd.x - lineStart.x;
		double dy = lineEnd.y - lineStart.y;

		//Normalise
		double mag = pow(pow(dx, 2.0) + pow(dy, 2.0), 0.5);
		if (mag > 0.0)
		{
			dx /= mag; dy /= mag;
		}

		double pvx = pt.x - lineStart.x;
		double pvy = pt.y - lineStart.y;

		//Get dot product (project pv onto normalized direction)
		double pvdot = dx * pvx + dy * pvy;

		//Scale line direction vector
		double dsx = pvdot * dx;
		double dsy = pvdot * dy;

		//Subtract this from pv
		double ax = pvx - dsx;
		double ay = pvy - dsy;

		return (real)pow(pow(ax, 2.0) + (real)pow(ay, 2.0), 0.5);
	}

	static vector<vec2> ramer_douglas_peucker_recursion(const vector<vec2>& points, real epsilon, s32 index, s32 end)
	{
		real max_dist = 0.0f;
		s32 next_index = index;

		for (s32 i = index + 1; i < end - 1; i++)
		{
			real dist = perpendicular_distance(points[index], points[end - 1], points[i]);
			if (dist > max_dist)
			{
				next_index = i;
				max_dist = dist;
			}
		}

		if (max_dist > epsilon)
		{
			auto a = ramer_douglas_peucker_recursion(points, epsilon, index, next_index);
			auto b = ramer_douglas_peucker_recursion(points, epsilon, next_index, end);

			a.insert(a.end(), b.begin(), b.end());
			return a;
		}
		else
		{
			vector<vec2> result;
			result.push_back(points[index]);
			result.push_back(points[end - 1]);
			return result;
		}
	}

	vector<vec2> ramer_douglas_peucker(const vector<vec2>& points, real epsilon)
	{
		auto result = ramer_douglas_peucker_recursion(points, epsilon, 0, (s32)points.size());
		TRACE(result.size());
		return result;
	}

	//LineDetectionAccumulator detect_lines(const vector<vec2>& points, const vec2& origin)
	//{
	//	LineDetectionAccumulator result;
	//
	//	AABB aabb;
	//
	//	for (s32 i = 0; i < points.size(); i++)
	//	{
	//		auto& point = points[i];
	//		aabb.fit_around_point(point);
	//	}
	//
	//	//real w = aabb.width();
	//	//real h = aabb.height();
	//
	//	//result.img_height = h;
	//	//result.img_width = w;
	//
	//	result.aabb = aabb;
	//
	//	//real hough_height = ((sqrt(2.0) * (h > w ? h : w)) / 2.0);
	//	//result.height = (s32)hough_height * 2.0;
	//	//result.width = 180;
	//	//result.accumulator = vector<s32>(result.width * result.height * 4, 0);
	//	//result.accumulator.clear();
	//
	//	for (s32 i = 0; i < points.size(); i++)
	//	{
	//		vec2 point = points[i];
	//		//point.x -= aabb.left;
	//		//point.y -= aabb.top;
	//
	//		//point.x = (point.x - aabb.left) / aabb.width();
	//		//point.y = (point.y - aabb.top) / aabb.height();
	//
	//		for (s32 t = 0; t < 180; t++)
	//		{
	//			//real r = point.x * cos(RADIANS(t)) + point.y * sin(RADIANS(t));
	//
	//			vec2 dir = vec2(cos(RADIANS(t)), sin(RADIANS(t)));
	//			real r = perpendicular_distance(point - dir * 100.0, point + dir * 100.0, vec2(0, 0)/*vec2(aabb.left + aabb.width() / 2, aabb.right + aabb.height() / 2)*/);
	//
	//			PolarCoordinate coord;
	//			coord.radius = (s32)round(r * 77);
	//			coord.theta = t;
	//
	//			result.accumulator[coord]++;
	//
	//			//s32 index = (s32)((round(r + hough_height) * 180.0)) + t;
	//			//
	//			//auto j = result.accumulator.find(index);
	//			//if (j != result.accumulator.end())
	//			//{
	//			//	(*j).second++;
	//			//}
	//			//
	//			//result.accumulator[index]++;
	//		}
	//	}
	//
	//	return result;
	//}
	//
	//vector<Line> LineDetectionAccumulator::get_lines(s32 threshhold)
	//{
	//	vector<Line> result;
	//
	//	map<s32, umap<PolarCoordinate, s32>::iterator> max;
	//
	//	for (auto i = accumulator.begin(); i != accumulator.end(); i++)
	//	{
	//		max[(*i).second] = i;
	//	}
	//
	//	auto end = max.rbegin();
	//	for (s32 i = 0; i < 100; i++)
	//	{
	//		if (end == max.rend())
	//		{
	//			break;
	//		}
	//
	//		auto& r = (*(*end).second).first.radius;
	//		auto& t = (*(*end).second).first.theta;
	//
	//		Line line;
	//
	//		//if (t >= 45 && t <= 135)
	//		//{
	//		//	(real)r* cos(RADIANS(t)) / sin(RADIANS(t));
	//		//}
	//		vec2 origin = vec2(aabb.left + aabb.width() / 2, aabb.top + aabb.height() / 2);
	//		line.p1 = /*origin + */vec2(cos(RADIANS(t)), sin(RADIANS(t))) * ((real)r / 1000.0);
	//
	//		//line.p1.x = line.p1.x * aabb.width() + aabb.left;
	//		//line.p1.y = line.p1.y * aabb.height() + aabb.top;
	//
	//		line.p2 = line.p1 + normalize(vec2(-line.p1.y, line.p1.x));
	//
	//		result.push_back(line);
	//
	//		end++;
	//	}
	//
	//	//for (auto i = accumulator.begin(); i != accumulator.end(); i++)
	//	//{
	//	//	if ((*i).second >= threshhold)
	//	//	{
	//	//		//auto& [r, t] = (*i).first;
	//	//		auto& r = (*i).first.radius;
	//	//		auto& t = (*i).first.theta;
	//
	//	//		Line line;
	//
	//	//		//if (t >= 45 && t <= 135)
	//	//		//{
	//	//		//	(real)r* cos(RADIANS(t)) / sin(RADIANS(t));
	//	//		//}
	//
	//	//		line.p1 = vec2(cos(RADIANS(t)), sin(RADIANS(t))) * (real)(r / 180.0);
	//
	//	//		line.p1.x = (line.p1.x) * aabb.width() + aabb.left;
	//	//		line.p1.y = (line.p1.y) * aabb.height() + aabb.top;
	//
	//	//		line.p2 = line.p1 + normalize(vec2(-line.p1.y, line.p1.x));
	//
	//	//		//line.p1.x += aabb.left;
	//	//		//line.p1.y += aabb.top;
	//
	//	//		//line.p2.x += aabb.left;
	//	//		//line.p2.y += aabb.top;
	//
	//	//		//if (t >= 45 && t <= 135)
	//	//		//{
	//	//		//	line.p1.x = 0;
	//	//		//	line.p1.y = ((real)(r - (height / 2)) - ((line.p1.x - (img_width / 2)) * cos(RADIANS(t)))) / sin(RADIANS(t)) + (img_height / 2);
	//	//		//	line.p2.x = img_width - 0;
	//	//		//	line.p2.y = ((real)(r - (height / 2)) - ((line.p2.x - (img_width / 2)) * cos(RADIANS(t)))) / sin(RADIANS(t)) + (img_height / 2);
	//	//		//}
	//	//		//else
	//	//		//{
	//	//		//	line.p1.y = 0;
	//	//		//	line.p1.x = ((double)(r - (height / 2)) - ((line.p1.y - (img_height / 2)) * sin(RADIANS(t)))) / cos(RADIANS(t)) + (img_width / 2);
	//	//		//	line.p2.y = img_height - 0;
	//	//		//	line.p2.x = ((double)(r - (height / 2)) - ((line.p2.y - (img_height / 2)) * sin(RADIANS(t)))) / cos(RADIANS(t)) + (img_width / 2);
	//	//		//}
	//
	//	//		result.push_back(line);
	//	//	}
	//	//}
	//
	//	//if (accumulator.empty())
	//	//{
	//	//	WARNING("Accumulator is empty!");
	//	//	return result;
	//	//}
	//	//
	//	//for (s32 r = 0; r < height; r++)
	//	//{
	//	//	for (s32 t = 0; t < width; t++)
	//	//	{
	//	//		auto& bin = accumulator[t + r * width];
	//	//		if (bin >= threshhold)
	//	//		{
	//	//			// add line to accumulator
	//	//
	//	//			//// TODO: impl this.
	//	//			//{
	//	//			//	int max = accumulator[(r * width) + t];
	//	//			//
	//	//			//	for (int ly = -4; ly <= 4; ly++)
	//	//			//	{
	//	//			//		for (int lx = -4; lx <= 4; lx++)
	//	//			//		{
	//	//			//			if ((ly + r >= 0 && ly + r < height) && (lx + t >= 0 && lx + t < width))
	//	//			//			{
	//	//			//				if ((int)accumulator[((r + ly) * width) + (t + lx)] > max)
	//	//			//				{
	//	//			//					max = accumulator[((r + ly) * width) + (t + lx)];
	//	//			//					ly = lx = 5;
	//	//			//				}
	//	//			//			}
	//	//			//		}
	//	//			//	}
	//	//			//
	//	//			//	if (max > (int)accumulator[(r * width) + t])
	//	//			//	{
	//	//			//		continue;
	//	//			//	}
	//	//			//}
	//	//
	//	//			Line line;
	//	//
	//	//			if (t >= 45 && t <= 135)
	//	//			{
	//	//				line.p1.x = 0;
	//	//				line.p1.y = ((real)(r - (height / 2)) - ((line.p1.x - (img_width / 2)) * cos(RADIANS(t)))) / sin(RADIANS(t)) + (img_height / 2);
	//	//				line.p2.x = img_width - 0;
	//	//				line.p2.y = ((real)(r - (height / 2)) - ((line.p2.x - (img_width / 2)) * cos(RADIANS(t)))) / sin(RADIANS(t)) + (img_height / 2);
	//	//			}
	//	//			else
	//	//			{
	//	//				line.p1.y = 0;
	//	//				line.p1.x = ((double)(r - (height / 2)) - ((line.p1.y - (img_height / 2)) * sin(RADIANS(t)))) / cos(RADIANS(t)) + (img_width / 2);
	//	//				line.p2.y = img_height - 0;
	//	//				line.p2.x = ((double)(r - (height / 2)) - ((line.p2.y - (img_height / 2)) * sin(RADIANS(t)))) / cos(RADIANS(t)) + (img_width / 2);
	//	//			}
	//	//
	//	//			result.push_back(line);
	//	//		}
	//	//	}
	//	//}
	//
	//	LOG(result.size());
	//
	//	return result;
	//}

}
