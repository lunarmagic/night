#pragma once

#include "node/Node2D.h"
#include "color/Color.h"
#include "utility.h"
#include "Polygon/Polygon.h"
//#include "Canvas.h"
#include "ComputeShader/ComputeShader.h"

#define PEN_BOX_DEBUG

namespace night
{

struct Canvas;

enum EBoxDrawingType
{
	NOT_A_BOX = 0,
	Y_BOX,
	I_BOX,
	T_BOX,
	SQ_BOX
};

struct BoxDrawingParams
{
	const vector<vector<vec2>>& lines;
	u8 should_evaluate_far_corners{ true };
	s32 rasterization_resolution{ 500 };
	real t_box_approximation_threshhold{ -1.0 };
};

//struct BoxDrawingScoreResult
//{
//	real score{ -1.0f };
//};

struct BoxDrawing : public Node2D
{
	BoxDrawing(const BoxDrawingParams& params);

	AABB aabb();

protected:

	virtual void on_render() override; // TODO: add debug_render

private:

	Polygon _contour;
	real _area{ 0.0 };
	
	struct BoxDrawingCanidate
	{
		real y_box_probability{ -1.0f };
		real i_box_probability{ -1.0f };
		real raster_score{ -1.0f };
		real canidate_probability{ -1.0f };

		//struct
		//{
		//	vec2 vp;
		//	real global_par;
		//	real local_par;
		//}vps[3];

		array<vec2, 3> vps;

		vector<Polygon> clips;
		EBoxDrawingType type;
		s32 index{ -1 };

		real view_angle;
		vec2 orthocenter;
	};

	array<BoxDrawingCanidate, 4> _canidates;


	u8 _should_evaluate_far_corners{ true }; // it is much easier to draw a box with only near corners.
	real _t_box_approximation_threshhold{ 0.02f };
	real _rasterization_clip_area_weight{ 1.0f };
	u8 _show_debug_info{ true };

	ref<ComputeShader> _compute_shader;
	ref<ComputeShader> _debug_shader;
	u8 _should_save_memory{ false };

	void approximate_countour();

	real evaluate_raster_score(const vector<Polygon>& clips);
	BoxDrawingCanidate evaluate_i_box_canidate(s32 index);
	BoxDrawingCanidate evaluate_y_box_canidate();
	BoxDrawingCanidate evaluate_t_box_canidate();

	void evaluate_score();
	
public:

	const array<BoxDrawingCanidate, 4> canidates() const { return _canidates; }
	const real& view_angle() const { return _canidates[0].view_angle; }
	const vec2& orthocenter() const { return _canidates[0].orthocenter; }
	const array<vec2, 3>& vanishing_points() const { return _canidates[0].vps; }
	const EBoxDrawingType& type() const { return _canidates[0].type; }
	const real& score() const { return _canidates[0].raster_score; }
};

}