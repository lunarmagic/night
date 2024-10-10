#pragma once

#include "CameraGameMode.h"

namespace night
{

	struct Box;
	struct BoxDrawing;
	struct Canvas;

	struct FreeformBoxesModeParams
	{
		u8 should_evaluate_far_corners{ true };
		s32 box_rasterization_resolution{ 500 };

		// variance weight.
	};

	struct FreeformBoxesMode : public CameraGameMode
	{
		FreeformBoxesMode(const FreeformBoxesModeParams& params = {});

		void should_evaluate_far_corners(u8 x) { _shouldEvaluateFarCorners = x; }
		u8 is_evaluating_far_corners() const { return _shouldEvaluateFarCorners; }

		void box_drawing_rasterization_resolution(s32 x) { _boxDrawingRasterizationResolution = x; }
		s32 box_drawing_rasterization_resolution() const { return _boxDrawingRasterizationResolution; }

	protected:

		void submit_box(); // TODO: return score
		void submit_box(ref<Box> box); // TODO: return score
		void clear_boxes();

		//void on_update(real delta) override;
		virtual void on_render() override; // debug

	private:

		u8 _isDrawing{ false };

		ref<Box> _testBox;

		ref<Canvas> _canvas;

		vector<ref<BoxDrawing>> _boxDrawings;	// todo : make a more generalized base struct and put these in a sub struct
		u8 _shouldEvaluateFarCorners{ true };
		s32 _boxDrawingRasterizationResolution{ 500 };

		real _averageViewAngle{ 0.0f };
		real _averageViewAngleWeighted{ 0.0f };
		real _viewAngleVariance{ 0.0f };
		real _viewAngleVarienceWeight{ 50.0f };

		vec2 _averageOrthocenter{ vec2(0) };
		vec2 _orthocenterVariance{ vec2(0) };
		real _orthocenterVarienceWeight{ 50.0f };

		real _score{ 0.0f };
		real _deltaScore{ 0.0f };

		void update_score();

		//  TODO: calculate the deviation of each view angle and ortho-center, less deviation = better score.
	};

}