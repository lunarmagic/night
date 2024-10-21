
#include "nightpch.h"
#include "math.h"
//#include "core.h"
#include "log/log.h"

namespace night
{

	real normalize_to_range(real value, real min, real max)
	{
		ASSERT(max - min != 0.0f);
		return (value - min) / (max - min);
	}

	real lerp(real a, real b, real t)
	{
		return a + (b - a) * t;
	}

	real dampen(real a, real b, real t, real delta)
	{
		t = 1.0f - pow(t, delta);
		return lerp(a, b, t);
	}

	real smoothstep(real a, real b, real t)
	{
		real v1 = t * t;
		real v2 = 1.0f - (1.0f - t) * (1.0f - t);
		return lerp(a, b, lerp(v1, v2, t));
	}

	vec2 bezier(const vec2& a, const vec2& b, const vec2& c, real t)
	{
		const real x = (1.f - t) * (1.f - t) * a.x + 2.f * (1.f - t) * t * b.x + t * t * c.x;
		const real y = (1.f - t) * (1.f - t) * a.y + 2.f * (1.f - t) * t * b.y + t * t * c.y;

		return { x, y };
	}

	vec2 slerp(const vec2& a, const vec2& b, real t)
	{
		real d = dot(a, b);
		d = CLAMP(d, -1.0, 1.0);
		real theta = acos(d) * t;
		vec2 relative = normalize(b - a * d);
		return (a * cos(theta)) + (relative * sin(theta));
	}

	real NIGHT_API angle(const vec2 v)
	{
		return atan2(v.x, v.y);
	}

	real angle_clockwise(const vec2& a, const vec2& b)
	{
		real d = a.x * b.x + a.y * b.y;
		real det = a.x * b.y - a.y * b.x;
		real angle = atan2(-det, -d) + R_PI;
		return angle;
	}
	real angle_counter_clockwise(const vec2& a, const vec2& b)
	{
		real d = a.x * b.x + -a.y * -b.y;
		real det = a.x * -b.y - -a.y * b.x;
		real angle = atan2(-det, -d) + R_PI;
		return angle;
	}

	real perp_dot(const vec2& a, const vec2& b)
	{
		return (a.y * b.x - a.x * b.y);
	}

	vec2 cross(const vec2& a, const vec2& b)
	{
		vec3 aa = { a.x, a.y, 0.0f };
		vec3 bb = { b.x, b.y, 0.0f };
		vec3 c1 = cross(aa, bb);

		return vec2{ c1.x, c1.y };
	}

	vec2 triple_cross(const vec2& a, const vec2& b, const vec2& c)
	{
		vec3 aa = { a.x, a.y, 0.0f };
		vec3 bb = { b.x, b.y, 0.0f };
		vec3 cc = { c.x, c.y, 0.0f };
		vec3 c1 = cross(aa, bb);
		vec3 c2 = cross(c1, cc);

		return vec2{ c2.x, c2.y };
	}

	vec3 triple_cross(const vec3& a, const vec3& b, const vec3& c)
	{
		vec3 c1 = cross(a, b);
		vec3 c2 = cross(c1, c);

		return c2;
	}

	LineProjectionResult project_lines(const vec2& a1, const vec2& a2, const vec2& b1, const vec2& b2)
	{
		const vec2 a = a2 - a1;
		const vec2 b = b2 - b1;
		const vec2 c = b2 - a2;
		const real f = perp_dot(a, b);

		return
		{
			.a = perp_dot(b, c),
			.b = perp_dot(a, c),
			.f = f
		};
	}

	RaycastResult raycast(const vec2& ray_origin, const vec2& ray_dir, const vec2& plane_origin, const vec2& plane_normal)
	{
		RaycastResult result;
		auto projection = project_lines(ray_origin, ray_origin + ray_dir, plane_origin, plane_origin + plane_normal);

		result.result = projection.is_valid() && projection.is_intersecting();
		result.t = projection.a_toi();
		result.contact = ray_origin + ray_dir * result.t;
	
		return result;
	}

	ProjectionResult project(const vec2& ray_origin, const vec2& ray_dir, const vec2& plane_origin, const vec2& plane_normal)
	{
		ProjectionResult result = {};

		//const vec2 a = -ray_dir;
		//const vec2 b = -plane_normal;
		//const vec2 c = plane_origin - ray_origin;
		//const real f = perp_dot(a, b);

		//if (f == 0.0f)
		//{
		//	return { .is_valid = false };
		//}

		//real aa = perp_dot(b, c);
		//real bb = perp_dot(a, c);

		//if (f < 0.f)
		//{
		//	if (aa > 0.f)     return { .is_valid = false };;
		//	if (bb > 0.f)     return { .is_valid = false };;
		//	if (aa < f)       return { .is_valid = false };;
		//	if (bb < f)       return { .is_valid = false };;
		//}
		//else
		//{
		//	if (aa < 0.f)     return { .is_valid = false };;
		//	if (bb < 0.f)     return { .is_valid = false };;
		//	if (aa > f)       return { .is_valid = false };;
		//	if (bb > f)       return { .is_valid = false };;
		//}

		//real t1 = 1.f - (aa / f);
		//real t2 = 1.f - (bb / f);

		//return
		//{
		//	.is_valid = true,
		//	.t1 = t1,
		//	.t2 = t2,
		//	.contact = ray_origin + ray_dir * t1
		//};

		//return
		//{
		//	.a = perp_dot(b, c),
		//	.b = perp_dot(a, c),
		//	.f = f
		//};

		auto projection = project_lines(ray_origin, ray_origin + ray_dir, plane_origin, plane_origin + plane_normal);

		result.is_valid = abs(projection.f) > 0.00001;
		if (result.is_valid)
		{
			result.t1 = projection.a_toi();
			result.t2 = projection.b_toi();
			result.contact = ray_origin + ray_dir * result.t1;
			result.f = projection.f;
		}

		return result;
	}

	//ProjectionResult3D project(const vec3& from, const vec3& to, const vec3& plane_origin, const vec3& plane_normal)
	//{
	//	ProjectionResult3D result;
	//
	//	vec3 ray_direction = to - from;
	//	vec3 difference = plane_origin - from;
	//	real product_1 = dot(difference, plane_normal);
	//	real product_2 = dot(ray_direction, plane_normal);
	//
	//	result.point = { product_1, product_2 };
	//	return result;
	//}

	vec3 raycast_to_plane(const vec3& from, const vec3& to, const vec3& plane_origin, const vec3& plane_normal)
	{
		vec3 ray_direction = to - from;
		vec3 difference = plane_origin - from;
		real product_1 = dot(difference, plane_normal);
		real product_2 = dot(ray_direction, plane_normal);
		real distance_from_origin_to_plane = product_1 / product_2;
		vec3 intersection = from + ray_direction * distance_from_origin_to_plane;
		return intersection;
	}

	RaycastSphereResult raycast_sphere(const vec3& ray_origin, const vec3& ray_direction, const vec3& sphere_origin, real sphere_radius)
	{
		RaycastSphereResult result = { .result = false }; // TODO: use sphere_origin

		real a = dot(ray_direction, ray_direction);
		real b = 2 * dot(ray_direction, sphere_origin - ray_origin);
		real c = dot(sphere_origin - ray_origin, sphere_origin - ray_origin);
		c -= sphere_radius * sphere_radius;

		real dt = b * b - 4 * a * c;

		if (dt >= 0) 
		{
			result.t0 = (-b - sqrt(dt)) / (a * 2);
			result.t1 = (-b + sqrt(dt)) / (a * 2);
			result.result = true;
		}

		return result;
	}

	ProjectPointToPlaneResult project_point_to_plane(const vec2& point, const vec2& plane_origin, const vec2& plane_normal)
	{
		real t = dot(point - plane_origin, plane_normal) / dot(plane_normal, plane_normal);
		vec2 projected_point = plane_origin + plane_normal * t;
		return {.point = projected_point, .t = t };
	}

	DistanceToLineResult distance_to_line(const vec2& line_a, const vec2& line_b, const vec2& point)
	{
		vec2 ab = line_b - line_a;
		real t = dot(point - line_a, ab) / dot(ab, ab);
		t = MIN(MAX(t, 0.0f), 1.0f);
		vec2 projected_point = line_a + ab * t;
		real dist = distance(point, projected_point);
		return { .distance = dist, .t = t };
	}

	DistanceToLineResult distance_to_plane(const vec2& line_a, const vec2& line_b, const vec2& point)
	{
		vec2 ab = line_b - line_a;
		real t = dot(point - line_a, ab) / dot(ab, ab);
		vec2 projected_point = line_a + ab * t;
		real dist = distance(point, projected_point);
		return { .distance = dist, .t = t };
	}

}