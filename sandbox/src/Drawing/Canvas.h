#pragma once

#include "utility.h"
#include "color/Color.h"
#include "node/Node2D.h"
#include "ComputeShader/ComputeShader.h"

#define CANVAS_DEFAULT_SMOOTHING 10

namespace night
{
	struct FragmentOverrideParams
	{
		const LineFragmentData& fragment;
		const vec2& p1;
		const vec2& p2;
	};

	struct CanvasParams
	{
		s32 smoothing{ CANVAS_DEFAULT_SMOOTHING };
		Color pen_color{ WHITE };
		function<void(const FragmentOverrideParams&)> fragment_callback{ nullptr };

		ComputeShaderParams compute_shader_params{};
	};

	struct Canvas : public Node2D
	{
		Canvas(const CanvasParams& params = {});

		void start_line();
		void end_line();

		void clear();

		const vector<vector<vec2>>& lines() const { return _lines; }

	protected:

		virtual void on_update(real delte) override;
		//virtual void on_render() override;

	private:

		Color _canvasColor{ BLACK };
		Color _lineColor{ WHITE };

		real _lineThickness{ 0.005f };
		real _lineExponent{ 2.0f };

		u8 _isDrawingLine{ false };
		vector<vector<vec2>> _lines;
		vector<vec2> _currentLine;

		s32 _smoothing{ CANVAS_DEFAULT_SMOOTHING }; // TODO: make this framerate independant
		deque<vec2> _smoothingBuffer;

		ref<ComputeShader> _computeShader;
		function<void(const FragmentOverrideParams&)> _fragmentCallback;

		void compute_line(const vec2& p1, const vec2& p2);
		// TODO: add pen pressure controls
	};

}