#pragma once

#include "IForm.h"
#include "color/Color.h"

namespace night
{

	struct SphereParams
	{
		const mat4& transform{ mat4(1) };
		const real& radius{ 1.0f };
		const Color& color{ BLACK };
	};

	struct Sphere : public IForm
	{
		Sphere(const SphereParams& params);

		void radius(real radius) { _radius = radius; }
		real radius() const { return _radius; }

	protected:

		virtual void on_render() override;

	private:

		real _radius{ 1.0f };
	};

}