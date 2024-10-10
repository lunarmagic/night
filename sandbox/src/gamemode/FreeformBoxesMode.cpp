
#include "nightpch.h"
#include "FreeformBoxesMode.h"
#include "Renderer3D/Renderer3D.h"
#include "Drawing/Canvas.h"
#include "form/Box.h"
#include "window/IWindow.h"
#include "math/math.h"
#include "Drawing/BoxDrawing.h"
#include "text/Text.h"
#include "points/Points.h"
#include "aabb/AABB.h"

namespace night
{

	FreeformBoxesMode::FreeformBoxesMode(const FreeformBoxesModeParams& params)
	{
		BIND_INPUT(EKey::APOSTROPHE, EInputType::PRESSED, [&]()
		{
			submit_box(_testBox);
		});

		BIND_INPUT(EKey::RETURN, EInputType::PRESSED, [&]()
		{
			submit_box();
		});

		BIND_INPUT(EKey::DELETE, EInputType::PRESSED, [&]()
		{
			clear_boxes();
		});

		_testBox = create<Box>("Test Box", BoxParams{ .transform = mat4(1), .extents = {1, 1, 1} });
		_canvas = create<Canvas>("Canvas");

		_shouldEvaluateFarCorners = params.should_evaluate_far_corners;
		_boxDrawingRasterizationResolution = params.box_rasterization_resolution;

		create<Text>("test text", TextParams{ .text = "TEST135y", .font = "Font16x", .position = vec2(-1.0f, -1.0f), .scale = vec2(0.15f) });
		create<Points>("points", PointsParams
			{
				.position = { vec2(-0.5f, 0.5) },
				.scale = { vec2(0.025f) },
				.amount = { 0.56f },
				.range = { 100.0f },
				.velocity = { vec2(0.0f, -1.0f) },
				.lifespan = { 0.5f },
				.font = { "Font10x" }
			}
		);
	}

	void FreeformBoxesMode::submit_box()
	{
		if (_canvas == nullptr)
		{
			return;
		}

		auto box_drawing = create<BoxDrawing>("Box Drawing", BoxDrawingParams{
				.lines = _canvas->lines(),
				.should_evaluate_far_corners = _shouldEvaluateFarCorners,
				.rasterization_resolution = _boxDrawingRasterizationResolution
			}
		);

		if (box_drawing != nullptr)
		{
			_boxDrawings.push_back(box_drawing);
		}

		_canvas->clear();

		update_score();
	}

	void FreeformBoxesMode::submit_box(ref<Box> box)
	{
		if (box == nullptr)
		{
			WARNING("box is nullptr");
			return;
		}

		vec2 points_proj[8];
		for (s32 i = 0; i < 8; i++)
		{
			points_proj[i] = Renderer3D::project_point_to_view_plane((box->transform() * box->points()[i]));
		}

		vector<vector<vec2>> lines;
		auto& front = lines.emplace_back(vector<vec2>());
		front.push_back(points_proj[0]);
		front.push_back(points_proj[1]);
		front.push_back(points_proj[2]);
		front.push_back(points_proj[3]);
		front.push_back(points_proj[0]);

		auto& back = lines.emplace_back(vector<vec2>());
		back.push_back(points_proj[4]);
		back.push_back(points_proj[5]);
		back.push_back(points_proj[6]);
		back.push_back(points_proj[7]);
		back.push_back(points_proj[4]);

		auto& side_1 = lines.emplace_back(vector<vec2>());
		side_1.push_back(points_proj[0]);
		side_1.push_back(points_proj[4]);

		auto& side_2 = lines.emplace_back(vector<vec2>());
		side_2.push_back(points_proj[1]);
		side_2.push_back(points_proj[5]);

		auto& side_3 = lines.emplace_back(vector<vec2>());
		side_3.push_back(points_proj[2]);
		side_3.push_back(points_proj[6]);

		auto& side_4 = lines.emplace_back(vector<vec2>());
		side_4.push_back(points_proj[3]);
		side_4.push_back(points_proj[7]);

		auto box_drawing = create<BoxDrawing>("Box Drawing", BoxDrawingParams{
			.lines = lines,
			.should_evaluate_far_corners = _shouldEvaluateFarCorners,
			.rasterization_resolution = _boxDrawingRasterizationResolution
			}
		);

		if (box_drawing != nullptr)
		{
			_boxDrawings.push_back(box_drawing);
		}

		update_score();
	}

	void FreeformBoxesMode::update_score()
	{
		if (_boxDrawings.empty())
		{
			WARNING("_boxDrawings is empty");
			return;
		}

		_viewAngleVariance = 0.0f;
		_averageViewAngle = 0.0f;
		_averageViewAngleWeighted = 0.0f;
		_orthocenterVariance = { 0.0f, 0.0f };
		_averageOrthocenter = { 0.0f, 0.0f };

		real _prev_score = _score;
		_score = 0.0f;

		for (s32 i = 0; i < _boxDrawings.size(); i++)
		{
			auto& box_drawing = _boxDrawings[i];
			if (box_drawing != nullptr)
			{
				_score += box_drawing->score();
				_averageViewAngleWeighted += log(box_drawing->view_angle() + 1.0f);
				_averageViewAngle += box_drawing->view_angle();
				_averageOrthocenter += box_drawing->orthocenter();
			}
		}

		_averageViewAngle /= _boxDrawings.size();
		_averageViewAngleWeighted /= _boxDrawings.size();
		_averageOrthocenter /= _boxDrawings.size();

		for (s32 i = 0; i < _boxDrawings.size(); i++)
		{
			auto& box_drawing = _boxDrawings[i];
			if (box_drawing != nullptr)
			{
				real view_angle_weighted = log(box_drawing->view_angle() + 1.0f);
				auto& orthocenter = box_drawing->orthocenter();

				real vw_var = ((view_angle_weighted - _averageViewAngleWeighted) * (view_angle_weighted - _averageViewAngleWeighted));
				vec2 o_var = ((orthocenter - _averageOrthocenter) * (orthocenter * _averageOrthocenter));

				_viewAngleVariance += vw_var * _viewAngleVarienceWeight + 1.0f;
				_orthocenterVariance += o_var * _orthocenterVarienceWeight;
			}
		}

		_viewAngleVariance /= _boxDrawings.size();
		_orthocenterVariance /= _boxDrawings.size();
		_score /= _viewAngleVariance;
		_deltaScore = _score - _prev_score;

		// spawn points object
		auto& back = _boxDrawings.back();

		if (back != nullptr)
		{
			AABB aabb = back->aabb();
			vec2 p = { aabb.left, aabb.top }; // TODO: add left-handed mode
			create<Points>("Points", PointsParams
				{
					.position = p,
					.scale = vec2{0.166f},
					.amount = _deltaScore
				}
			);
		}

		TRACE("score: ", _score);
		TRACE("delta score: ", _deltaScore);
		TRACE("view_angle_variance: ", _viewAngleVariance);
		TRACE("average view angle: ", DEGREES(_averageViewAngle));
		TRACE("ortho_center var: ", length(_orthocenterVariance));
	}

	void FreeformBoxesMode::clear_boxes()
	{
		for (const auto& i : _boxDrawings)
		{
			if (i != nullptr)
			{
				remove(i->name());
			}
		}

		_boxDrawings.clear();
		_canvas->clear();

		_score = 0.0f;
		_deltaScore = 0.0f;

		_viewAngleVariance = 0.0f;
		_averageViewAngle = 0.0f;
		_averageViewAngleWeighted = 0.0f;
		_orthocenterVariance = { 0.0f, 0.0f };
		_averageOrthocenter = { 0.0f, 0.0f };
	}

	void FreeformBoxesMode::on_render()
	{
		utility::renderer().draw_point(_averageOrthocenter, WHITE);
	}

}