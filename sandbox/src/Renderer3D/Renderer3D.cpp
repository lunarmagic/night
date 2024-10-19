
#include "nightpch.h"
#include "Renderer3D.h"
#include "math/math.h"

namespace night
{

mat4 Renderer3D::_mvp = mat4(1.0f);
real Renderer3D::_view_angle = RADIANS(60.0f);
EPerspectiveMode Renderer3D::_mode = EPerspectiveMode::Linear;

vec4 Renderer3D::project_point_to_view_plane(const vec4& point)
{
	switch (_mode)
	{
	case EPerspectiveMode::Linear:
	{
		return project_point_linear(point);
		break;
	}
	case EPerspectiveMode::Semi_Curvilinear:
	{
		return project_point_semi_curvilinear(point);
		break;
	}
	case EPerspectiveMode::Parallel:
	{
		return project_point_parallel(point);
		break;
	}
	}

	return {};
}

constexpr vec3 _forward = { 0.0f, 0.0f, 1.0f };
constexpr vec3 _up = { 0.0f, -1.0f, 0.0f };
constexpr vec3 _right = { 1.0f, 0.0f, 0.0f };

u8 Renderer3D::should_cull_face(const vec4& p1, const vec4& p2, const vec4& p3)
{
	vec2 pp1 = project_point_to_view_plane(p1);
	vec2 pp2 = project_point_to_view_plane(p2);
	vec2 pp3 = project_point_to_view_plane(p3);

	return (perp_dot(pp2 - pp1, pp3 - pp2) >= 0.0f);
}

vec4 Renderer3D::project_point_linear(const vec4& point)
{
	constexpr vec3 plane_origin = { 0.0f, 0.0f, 0.0f };
	vec3 plane_normal = { 0.0f, 0.0f, 1.0f };
	vec3 from = _mvp * point;
	vec3 to = { plane_origin.x, plane_origin.y, -_view_angle };

	vec3 ray_direction = to - from;
	vec3 difference = plane_origin - from;
	real d1 = dot(difference, plane_normal);
	real d2 = dot(ray_direction, plane_normal);
	real dist = d1 / d2;
	vec3 intersection = from + ray_direction * dist;

	return { intersection.x, intersection.y, dist, 1.0f };
}

vec4 Renderer3D::project_point_semi_curvilinear(const vec4& p) // TODO: remove this
{
	vec4 result(0);

	const vec3 point = (vec3)p;
	const vec3 eyeball_position = { 0.0f, 0.0f, -_view_angle * R_PI * 2.5};
	const real eyeball_radius = _view_angle * R_PI * 2.5;
	const vec3 ray_direction = (eyeball_position - point);

	auto hit = raycast_sphere(point, ray_direction, eyeball_position, eyeball_radius);

	if (hit.result)
	{
		vec3 contact = point + ray_direction * hit.t0;
		result.x = dot(_right, normalize(contact));
		result.y = dot(_up, normalize(contact));
		result.z = distance(point, contact);
		result.w = 1.0f;
	}

	return result;
}

vec4 Renderer3D::project_point_parallel(const vec4& point)
{
	vec3 from = _mvp * point;
	return { from.x * 0.3f * _view_angle, from.y * 0.3f * _view_angle, from.z, 1.0f };
}

vec4 Renderer3D::inverse_project_point_to_view_plane(const vec2& point)
{
	switch (_mode)
	{
	case EPerspectiveMode::Linear:
	{
		vec3 vp_on_projection_plane = { point.x, point.y, 0.0f };
		vec3 eye = { 0.0f, 0.0f, -_view_angle };
		vec3 direction = normalize(vp_on_projection_plane - eye);
		return inverse(_mvp) * vec4 { direction.x, direction.y, direction.z, 0.0f };
		break;
	}
	case EPerspectiveMode::Semi_Curvilinear:
	{
		TRACE("TODO: IMPL");
		break;
	}
	}

	return {};
}

void Renderer3D::camera(const mat4& camera)
{
	_mvp = camera;
}

void Renderer3D::view_angle(real view_angle)
{
	_view_angle = view_angle;
}

}

