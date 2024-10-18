#pragma once

#include "Box.h"

namespace night
{

	struct IForm;
	//struct Box;

	struct Intersection
	{
		ref<IForm> a;
		ref<IForm> b;

		struct Line
		{
			vec3 p1;
			vec3 p2;
			//vec3 normal1;
			//vec3 normal2;
			Box::Plane plane1;
			Box::Plane plane2;
		};

		vector<Line> lines;

		static Intersection intersect(ref<Box> a, ref<Box> b); // TODO: use polymorphism
		static Intersection intersect_test(ref<Box> a, vec3 a_pos, ref<Box> b, vec3 b_pos); // TODO: use polymorphism
	};

}