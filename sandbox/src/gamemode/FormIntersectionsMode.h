#pragma once

#include "CameraGameMode.h"
#include "ComputeShader/ComputeShader.h"

namespace night
{

	struct Box;
	struct Canvas;
	struct Intersection;
	
	enum class EForm : u32
	{
		Box = BIT(1),
		Sphere = BIT(2),
		Cylinder = BIT(3),
		Cone = BIT(4),
		Pyramid = BIT(5)
	};

	struct FormIntersectionsModeParams
	{
		s32 form_count{ 5 };
		EForm which_forms{ 0xFFFFFFFF };
		ivec2 density{ 2, 2 };
		vec3 area{ 2, 2, 0 };
		vec3 variation{ 1.25f, 1.25f, 1.25f };

		ivec2 internal_resolution{ 1000, 1000 };
	};

	struct FormIntersectionsMode : public CameraGameMode
	{
		FormIntersectionsMode(const FormIntersectionsModeParams& params);
		
	protected:

		virtual void on_render() override;

		void submit();
		void reset();

	private:

		void spawn_forms();
		void render_wireframe();
		void render_intersections();
		void clear_forms();

		FormIntersectionsModeParams _params;
		vector<ref<Box>> _forms;
		vector<Intersection> _intersections;
		ref<ComputeShader> _computeShader;
		ref<ComputeShader> _wireframe;
		ref<Canvas> _canvas;
		// canvas, rotated view.

		Color8 _intersectionColor{ RED };
		Color8 _contoursColor{ WHITE };
		Color8 _wireframeColor{ WHITE / 4 };
		Color _canvasPenColor{ WHITE };

		real _wireframeThickness{ COMPUTE_SHADER_LINE_DEFAULT_THICKNESS * 1.5f };
		real _wireframeThicknessDepth{ 0.5f };

		u8 cull_normal(const vec3& normal, const vec3& point_on_plane);
	};

}