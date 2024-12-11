#pragma once

#include "CameraGameMode.h"
#include "ComputeShader/ComputeShader.h"
#include "FormIntersectionsMode.h"

//#define FORM_INTERSECTION_DEPTH_BUFFER_RESOLUTION 16

namespace night
{

	struct FormIntersectionsParallelModeParams
	{
		EForm which_forms{ 0xFFFFFFFF };
		ivec2 density{ 2, 2 };
		vec2 area{ 2, 2 };
		vec2 variation{ 1.25f, 1.25f };

		ivec2 internal_resolution{ 500, 500 };
	};

	struct FormIntersectionsParallelMode : public CameraGameMode
	{
		FormIntersectionsParallelMode(const FormIntersectionsParallelModeParams& params);

		real score() const { return _score; }

	protected:

		virtual void on_render() override;

		void submit();
		void reset();

	private:

		void spawn_forms();
		void clear_forms();

		FormIntersectionsParallelModeParams _params;
		vector<ref<Box>> _forms;
		vector<vector<Intersection>> _debug_intersections;

#define INTERSECTION_COVERAGE_BOUND_MIN -1
#define INTERSECTION_COVERAGE_BOUND_MAX 1

		struct IntersectionCoverageBound
		{
			real t;
			s32 bound;

			struct cmp
			{
				u8 operator() (IntersectionCoverageBound a, IntersectionCoverageBound b) const
				{
					if (a.t == b.t)
					{
						return (a.bound < b.bound);
					}
					else
					{
						return (a.t < b.t);
					}
				}
			};
		};

		struct FormIntersections
		{
			ref<Box> form_a;
			ref<Box> form_b;

			struct Intersection
			{
				s32 plane_a;
				s32 plane_b;

				Polygon area;
				vec3 origin;
				vec3 normal;
				real slope;

				//std::multiset<IntersectionCoverageBound, IntersectionCoverageBound::cmp> area_coverage;
				//array<real, 16> depth_buffer; // TODO: use area coverage.
			};

			vector<Intersection> intersections;
			real average_toi{ INFINITY };
			real distance_to_pixels{ INFINITY };
		};

		vector<FormIntersections> _intersections;
		ref<ComputeShader> _instersectionsDebugView;
		ref<ComputeShader> _unsignedDistanceField;

		FormIntersections intersect(ref<Box> form_a, ref<Box> form_b);
		real time_of_intersection(const vec2& point_on_overlapping_forms, const FormIntersections::Intersection& intersection);

		// returns a line segment of the intersection clamped between the 2 planes
		pair<vec2, vec2> intersection_of_time(real toi, const FormIntersections::Intersection& intersection);

		ref<Canvas> _canvas;

		Color8 _wireframeColor{ WHITE / 2 };
		Color _canvasPenColor{ BLACK };

		real _score{ -1.0f };
		u8 _is_submitted{ false };

		real _intersectionVarianceWeight{ 2.5f };
		real _intersectionVarianceExponent{ 1.0f };
	};

}