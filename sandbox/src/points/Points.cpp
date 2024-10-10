
#include "nightpch.h"
#include "points/Points.h"

namespace night
{
	Points::Points(const PointsParams& params)
		: Text(
			{
				.font = params.font,
				.position = params.position,
				.scale = params.scale
			}
		)
	{
		s32 points = s32(round(params.amount * params.range));
		text(to_string(points));

		if (points > (params.range * 0.9f))
		{
			color(WHITE);
		}
		else if (points > (params.range * 0.7f))
		{
			color(GREEN);
		}
		else if (points > (params.range * 0.5f))
		{
			color(YELLOW);
		}
		else if (points > (params.range * 0.3f))
		{
			color(ORANGE);
		}
		else
		{
			color(RED);
		}

		lifespan(params.lifespan);
		_amount = params.amount;
		_range = params.range;
		_velocity = params.velocity;
	}

	void Points::on_update(real delta)
	{
		//position(position() + _velocity * delta);

		translate(_velocity * delta);
		//rotate(delta * 5.0f);

		//mat4 x = transform();
		//x = translate(x, vec3(_velocity * delta, 0.0f));
		//transform(x);
	}
}