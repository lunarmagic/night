
#include "nightpch.h"
#include "Canvas.h"
#include "window/IWindow.h"
#include "BoxDrawing.h"
#include "Line.h"
#include "form/Box.h"
#include "Renderer3D/Renderer3D.h"
#include "ComputeShader/ComputeShader.h"
#include "algorithm/algorithm.h"
#include "math/math.h"

namespace night
{
	Canvas::Canvas(const CanvasParams& params)
	{
		_smoothing = params.smoothing;
		_lineColor = params.pen_color;
		_fragmentCallback = params.fragment_callback;

		// TODO: add action mapping and use actions "Begin Drawing" and "End Drawing"
		BIND_INPUT(EMouse::LEFT, EInputType::PRESSED, [&]()
		{
			start_line();
		});

		BIND_INPUT(EMouse::LEFT, EInputType::RELEASED, [&]()
		{
			end_line();
		});

		_computeShader = create<ComputeShader>("Canvas Compute Shader", params.compute_shader_params);
	}

	void Canvas::start_line()
	{
		_isDrawingLine = true;
	}

	void Canvas::end_line()
	{
		if (_smoothing >= 0)
		{
			while (!_smoothingBuffer.empty())
			{
				vec2 sum = vec2(0);

				s32 i = 0;
				for (; i < _smoothingBuffer.size(); i++)
				{
					sum += _smoothingBuffer[i];
				}

				sum /= i;

				if (!_currentLine.empty())
				{
					compute_line(_currentLine.back(), sum);
				}
				
				_currentLine.push_back(sum);
				_smoothingBuffer.pop_front();
			}
		}

		_lines.push_back(_currentLine);
		_currentLine.clear();
		_isDrawingLine = false;
	}

	void Canvas::on_update(real delte)
	{
		vec2 mouse = utility::window().mouse();

		if (!_isDrawingLine)
		{
			return;
		}

		if (_smoothing >= 0)
		{
			_smoothingBuffer.push_back(mouse);

			vec2 sum = vec2(0.0);
			s32 i = 0;

			for (; i < _smoothingBuffer.size(); i++)
			{
				auto& point = _smoothingBuffer[i];
				sum += point;
			}

			sum /= i;

			if (_smoothingBuffer.size() > _smoothing)
			{
				_smoothingBuffer.pop_front();
			}

			if (!_currentLine.empty())
			{
				if (sum == _currentLine.back())
				{
					return;
				}

				compute_line(_currentLine.back(), sum);
			}

			_currentLine.push_back(sum);
		}
		else
		{
			if (!_currentLine.empty())
			{
				if (mouse == _currentLine.back())
				{
					return;
				}

				compute_line(_currentLine.back(), mouse);
			}

			_currentLine.push_back(mouse);
		}
	}

	void Canvas::on_render() // TODO: change this to debug render
	{
		//for (s32 i = 0; i < _lines.size(); i++)
		//{
		//	auto& line = _lines[i];
		//	for (s32 j = 0; j < (s32)line.size() - 1; j++)
		//	{
		//		auto& p1 = line[j];
		//		auto& p2 = line[j + 1];
		//
		//		utility::renderer().draw_line(p1, p2, _lineColor);
		//	}
		//}
		//
		//for (s32 i = 0; i < (s32)_currentLine.size() - 1; i++)
		//{
		//	auto& p1 = _currentLine[i];
		//	auto& p2 = _currentLine[i + 1];
		//
		//	Window::get().draw_line(p1, p2, _lineColor);
		//}
	}

	void Canvas::compute_line(const vec2& p1, const vec2& p2)
	{
		if (_computeShader == nullptr)
		{
			ERROR("Canvas _computeShader is nullptr! name: ", name());
			return;
		}

		_computeShader->rasterize_line(p1, p2, [&](auto& fragment)
		{
			if (fragment.pixel != nullptr)
			{
				*fragment.pixel = Color8(_lineColor);
			}
		});

		//_computeShader->draw_line(
		//	{
		//		.p1 = p1,
		//		.p2 = p2,
		//		.thickness1 = _lineThickness,
		//		.thickness2 = _lineThickness,
		//		.exponent = _lineExponent,
		//		.color1 = _lineColor,
		//		.color2 = _lineColor
		//	}
		//);

		//Polygon polygon;
		//
		//vec2 normal = normalize(p2 - p1) * (_lineThickness/* * 2*/);
		//vec2 perpendicular = vec2(-normal.y, normal.x);
		//
		//vec2 p1a = p1 + (-normal - perpendicular);
		//vec2 p1b = p1 + (-normal + perpendicular);
		//
		//vec2 p2a = p2 + (normal + perpendicular);
		//vec2 p2b = p2 + (normal - perpendicular);
		//
		//polygon.push_back(p1a);
		//polygon.push_back(p1b);
		//polygon.push_back(p2a);
		//polygon.push_back(p2b);
		//
		//Color8 color8 = Color8(_lineColor);

		//_computeShader->rasterize_polygon(polygon, [&](auto& fragment)
		//{
		//	if (fragment.pixel != nullptr)
		//	{
		//		real dist = distance_to_line(p1, p2, _computeShader->internal_to_global(fragment.coordinate));
		//		dist = CLAMP(dist / _lineThickness, 0.0f, 1.0f);
		//		dist = 1.0f - exp(dist * _lineExponent) / exp(_lineExponent);

		//		if (dist < 1.0f) // multiply
		//		{
		//			fragment.pixel->a = MAX(fragment.pixel->a, (u8)(dist * 255.0f));

		//			fragment.pixel->r = color8.r; // TODO: may want to blend colors
		//			fragment.pixel->g = color8.g;
		//			fragment.pixel->b = color8.b;
		//		}
		//	}
		//});

		//if (!_fragmentCallback)
		//{
		//	_computeShader->rasterize_line(p1, p2, [&](auto& fragment)
		//	{
		//		if (fragment.pixel != nullptr)
		//		{
		//			*fragment.pixel = Color8(_lineColor);
		//		}
		//	});
		//}
		//else
		//{
		//	_computeShader->rasterize_line(p1, p2, [&](auto& fragment)
		//	{
		//		if (fragment.pixel != nullptr)
		//		{
		//			*fragment.pixel = Color8(_lineColor);
		//			_fragmentCallback({ .fragment = fragment, .p1 = p1, .p2 = p2 });
		//		}
		//	});
		//}
	}

	void Canvas::clear()
	{
		_computeShader->fill(0x00);
		_lines.clear();
		_currentLine.clear();
		_smoothingBuffer.clear();
		_isDrawingLine = false;
	}

}