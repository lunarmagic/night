#pragma once

#include "window/IWindow.h"

namespace night
{
	enum class EPerspectiveMode
	{
		Linear,
		Parallel,
		Semi_Curvilinear
	};

	struct Renderer3D
	{
		static vec4 project_point_to_view_plane(const vec4& point);

		static vec4 inverse_project_point_to_view_plane(const vec2& point);

		static void camera(const mat4& camera);

		static void view_angle(real view_angle);

		static void mode(EPerspectiveMode mode) { _mode = mode; }
		static EPerspectiveMode mode() { return _mode; }

		static u8 should_cull_face(const vec4& p1, const vec4& p2, const vec4& p3);

	private:

		static vec4 project_point_linear(const vec4& point);
		static vec4 project_point_semi_curvilinear(const vec4& point);
		static vec4 project_point_parallel(const vec4& point);

		static mat4 _mvp;
		static real _view_angle;
		static EPerspectiveMode _mode;
	};

}

