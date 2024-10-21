#pragma once

#include "log/log.h"

namespace night
{

	struct NIGHT_API gjk
	{
		// push first vertex of simplex, also get direction to origin.
		struct Simplex
		{
			array<vec2, 3> vertices{};
			s32 count{ 0 };

			u8 is_line() const { return count == 2; }
			u8 is_triangle() const { return count == 3; }

			void make_point(const vec2& p)
			{
				vertices[0] = p;
				count = 1;
			}

			void make_line(const vec2& a, const vec2& b)
			{
				vertices[0] = a;
				vertices[1] = b;
				count = 2;
			};

			void make_triangle(const vec2& a, const vec2& b, const vec2& c)
			{
				vertices[0] = a;
				vertices[1] = b;
				vertices[2] = c;
				count = 3;
			}

			constexpr void push_vertex(const vec2& vertex)
			{
				ASSERT(count < 3);
				vertices[count] = vertex;
				count++;
			}

			constexpr void pop_vertex()
			{
				ASSERT(count > 0);
				count--;
			}
		};

		static vec2 support(const vec2* vertices, s32 count, const vec2& direction);
		static u8 overlap(const vec2* vertices_a, s32 count_a, const vec2* vertices_b, s32 count_b);

		//bool overlap(const sdlfw::ProxyShape& convex_a, const sdlfw::ProxyShape& convex_b, const sdlfw::Transform& xa, const sdlfw::Transform& xb);
	};

}