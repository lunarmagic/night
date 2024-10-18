#pragma once

#include "CameraGameMode.h"
#include "ComputeShader/ComputeShader.h"
#include "FormIntersectionsMode.h"

namespace night
{

	//struct Box;
	//struct Canvas;
	//struct Intersection;

	//enum class EForm : u32
	//{
	//	Box = BIT(1),
	//	Sphere = BIT(2),
	//	Cylinder = BIT(3),
	//	Cone = BIT(4),
	//	Pyramid = BIT(5)
	//};

	struct FormIntersectionsParallelModeParams
	{
		//s32 form_count{ 5 };
		EForm which_forms{ 0xFFFFFFFF };
		ivec2 density{ 2, 2 };
		vec2 area{ 2, 2/*, 0*/ };
		vec2 variation{ 1.25f, 1.25f/*, 1.25f*/ };

		ivec2 internal_resolution{ 1000, 1000 };
	};

	struct FormIntersectionsParallelMode : public CameraGameMode
	{
		FormIntersectionsParallelMode(const FormIntersectionsParallelModeParams& params);

	protected:

		virtual void on_render() override;

		void submit();
		void reset();

	private:

		u8 cull_normal(const vec3& normal, const vec3& point_on_plane);
		void spawn_forms();
		//void render_wireframe();
		//void render_intersections();
		void clear_forms();

		FormIntersectionsParallelModeParams _params;
		vector<ref<Box>> _forms;
		vector<vector<Intersection>> _debug_intersections;

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
			};

			vector<Intersection> intersections;
		};

		vector<FormIntersections> _intersections;
		ref<ComputeShader> _instersectionsDebugView;

		FormIntersections intersect(ref<Box> form_a, ref<Box> form_b);

		//ref<ComputeShader> _computeShader;
		//ref<ComputeShader> _wireframe;
		ref<Canvas> _canvas;
		// canvas, rotated view.

		//Color8 _intersectionColor{ RED };
		//Color8 _contoursColor{ WHITE };
		Color8 _wireframeColor{ WHITE / 2 };
		Color _canvasPenColor{ BLACK };

		//real _wireframeThickness{ COMPUTE_SHADER_LINE_DEFAULT_THICKNESS * 1.5f };
		//real _wireframeThicknessDepth{ 0.5f };

		//u8 cull_normal(const vec3& normal, const vec3& point_on_plane);
	};

}