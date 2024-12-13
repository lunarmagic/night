#pragma once

#include "node/Node3D.h"
#include "color/Color.h"

namespace night
{
	//struct Line;
	enum class EFormType : u32
	{
		None = 0,
		Box = BIT(0),
		Sphere = BIT(1),
		Cylinder = BIT(2),
		Cone = BIT(3),
		Pyramid = BIT(4)
	};

	struct IForm : public Node3D // TODO: make Node3D
	{
		IForm(EFormType type);

		EFormType type() const { return _type; }

		// TODO: add contour function for optimizing pixel overlap detection.

		void color(const Color& color) { _color = color; }
		const Color& color() const { return _color; }

	private:

		EFormType _type;
		Color _color{ BLACK };
	};
}