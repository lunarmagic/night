#pragma once

#include "text/Text.h"

namespace night
{

	struct PointsParams
	{
		const vec2& position{ vec2(0) }; // TODO: transform2D
		const vec2& scale{ vec2(0.033f) };
		const real& amount{ -1.0f };
		const real& range{ 100.0f };
		const vec2& velocity{ vec2(0.0f, -0.33f) };
		const real& lifespan{ 0.66f };
		const std::string& font{ "Font16x" };
	};

	struct Points : public Text
	{
		Points(const PointsParams& params);

	protected:

		void on_update(real delta) override;

	private:

		real _amount{ -1.0f };
		real _range{ 100.0f };
		vec2 _velocity{ vec2(0.0f, -1.0f) };
		//real _lifespan{ 0.5f }; // TODO: add timestamp and lifespan to INode
	};

}