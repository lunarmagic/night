#pragma once

//#include "utility.h"
#include "core.h"

namespace night
{

	#ifdef NIGHT_USE_DOUBLE_PRECISION
	#define R_PI	3.14159265358979323846
	#else
	#define R_PI	3.14159265358979323846f
	#endif

	#define RADIANS(x) glm::radians((float)x)
	#define DEGREES(x) glm::degrees((float)x)

	real NIGHT_API normalize_to_range(real value, real min, real max);

	real NIGHT_API lerp(real a, real b, real t);

	real NIGHT_API dampen(real a, real b, real t, real delta);

	real NIGHT_API smoothstep(real a, real b, real t);

	vec2 NIGHT_API bezier(const vec2& a, const vec2& b, const vec2& c, real t);

	vec2 NIGHT_API slerp(const vec2& a, const vec2& b, real t);

	real NIGHT_API angle(const vec2 v);

	real NIGHT_API angle_clockwise(const vec2& a, const vec2& b);

	real NIGHT_API angle_counter_clockwise(const vec2& a, const vec2& b);

	real NIGHT_API perp_dot(const vec2& a, const vec2& b);

	vec2 NIGHT_API cross(const vec2& a, const vec2& b);

	vec2 NIGHT_API triple_cross(const vec2& a, const vec2& b, const vec2& c);

	vec3 NIGHT_API triple_cross(const vec3& a, const vec3& b, const vec3& c);

	struct LineProjectionResult
	{
		real a;
		real b;
		real f;

		real a_toi() const { return 1.f - (a / f); }
		real b_toi() const { return 1.f - (b / f); }

		u8 is_valid() const { return f != 0.0f; } // if the ray and plane are tangent, t will be infinite. and thus invalid

		u8 is_intersecting() const
		{
			if (f == 0.0f) return false;
			else if (f < 0.f)
			{
				if (a > 0.f)     return false;
				if (b > 0.f)     return false;
				if (a < f)       return false;
				if (b < f)       return false;
			}
			else
			{
				if (a < 0.f)     return false;
				if (b < 0.f)     return false;
				if (a > f)       return false;
				if (b > f)       return false;
			}

			return true;
		}

		u8 a_voronoi() const
		{
			if (f < 0.f)
			{
				if (a > 0.f)     return true;
				if (a < f)       return true;
			}
			else
			{
				if (a < 0.f)     return true;
				if (a > f)       return true;
			}

			return false;
		}

		u8 b_voronoi() const
		{
			if (f < 0.f)
			{
				if (b > 0.f)     return true;
				if (b < f)       return true;
			}
			else
			{
				if (b < 0.f)     return true;
				if (b > f)       return true;
			}

			return false;
		}

	};

	LineProjectionResult NIGHT_API project_lines(const vec2& a1, const vec2& a2, const vec2& b1, const vec2& b2);

	struct RaycastResult
	{
		u8 result;
		real t;
		vec2 contact;
	};

	RaycastResult NIGHT_API raycast(const vec2& ray_origin, const vec2& ray_dir, const vec2& plane_origin, const vec2& plane_normal); // TODO: make raycast file.

	struct ProjectionResult
	{
		u8 is_valid;
		real t1;
		real t2;
		vec2 contact;
		real f;
	};

	ProjectionResult NIGHT_API project(const vec2& ray_origin, const vec2& ray_dir, const vec2& plane_origin, const vec2& plane_normal);

	//struct ProjectionResult3D
	//{
	//	u8 is_valid;
	//	vec2 point;
	//};

	//ProjectionResult3D project(const vec3& from, const vec3& to, const vec3& plane_origin, const vec3& plane_normal);

	vec3 NIGHT_API raycast_to_plane(const vec3& from, const vec3& to, const vec3& plane_origin, const vec3& plane_normal); // TODO: better implement this.

	struct RaycastSphereResult
	{
		u8 result{ false };
		real t0;
		real t1;
		// n1 n2
	};

	RaycastSphereResult NIGHT_API raycast_sphere(const vec3& ray_origin, const vec3& ray_direction, const vec3& sphere_origin, real sphere_radius);

	struct DistanceToLineResult
	{
		real distance{ -1.0f };
		real t{ -1.0f };
	};

	DistanceToLineResult NIGHT_API distance_to_line(const vec2& line_a, const vec2& line_b, const vec2& point);
}