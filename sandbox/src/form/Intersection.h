#pragma once

namespace night
{

	struct IForm;
	struct Box;

	struct Intersection
	{
		ref<IForm> a;
		ref<IForm> b;

		struct Line
		{
			vec3 p1;
			vec3 p2;
			vec3 normal1;
			vec3 normal2;
		};

		vector<Line> lines;

		static Intersection intersect(ref<Box> a, ref<Box> b); // TODO: use polymorphism
	};

}