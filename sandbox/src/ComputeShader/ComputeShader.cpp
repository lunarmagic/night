
#include "nightpch.h"
#include "ComputeShader.h"
#include "algorithm/algorithm.h"
#include "math/math.h"
#include "Renderer3D/Renderer3D.h"

namespace night
{

	//u64 ComputeShader::_curr_id = 0;

	//u8 ComputeShader::init(const ComputeShaderParams& params)
	//{
	//	_curr_id++;
	//
	//	_unique_id = "ComputeShader" + to_string(_curr_id);
	//
	//	_surface = Window::get().create_surface(_unique_id, { .width = params.width, .height = params.height }); // TODO: use node uid for surf id
	//	if (_surface != nullptr)
	//	{
	//		_quad = params.quad;
	//		Window::get().create_texture(_unique_id, { .width = _surface->width(), .height = _surface->height(), .surface = _surface });
	//		return true;
	//	}
	//
	//	return false;
	//}

	//void ComputeShader::clean()
	//{
	//	TRACE("Destroying ComputeShader, id: ", _window_id);
	//	Window::get().destroy_surface(_window_id);
	//	Window::get().destroy_texture(_window_id);
	//	_window_id = "";
	//}

	ComputeShader::ComputeShader(const ComputeShaderParams& params)
	{
		_window_id = "ComputeShader" + to_string(unique_id());

		_surface = utility::window().create_surface(_window_id, { .width = params.width, .height = params.height });

		if (_surface != nullptr)
		{
			_quad = params.quad;
			_texture = utility::window().create_texture(_window_id, { /*.width = _surface->width(), .height = _surface->height(),*/ .surface = _surface });
		}
	}

	ComputeShader::~ComputeShader()
	{
		utility::window().destroy_surface(_window_id);
		utility::window().destroy_texture(_window_id);
	}

	void ComputeShader::fill(const Color& color)
	{
		if (_surface != nullptr)
		{
			Color8 color8 = Color8(color);

			Color8* pixels = _surface->pixels();

			for (s32 y = 0; y < _surface->height(); y++)
			{
				for (s32 x = 0; x < _surface->width(); x++)
				{
					pixels[x + y * _surface->width()] = color8;
				}
			}

			update_texture();
		}
		else
		{
			WARNING("ComputeShader surface is nullptr!");
		}
	}

	void ComputeShader::fill(const Color8& color) // TODO: optimize this
	{
		if (_surface != nullptr)
		{
			Color8* pixels = _surface->pixels();

			for (s32 y = 0; y < _surface->height(); y++)
			{
				for (s32 x = 0; x < _surface->width(); x++)
				{
					pixels[x + y * _surface->width()] = color;
				}
			}

			update_texture();
		}
		else
		{
			WARNING("ComputeShader surface is nullptr!");
		}
	}

	void ComputeShader::fill(u8 byte)
	{
		if (_surface != nullptr)
		{
			Color8* pixels = _surface->pixels();

			memset(pixels, byte, _surface->width() * _surface->height() * sizeof(Color8));

			update_texture();
		}
		else
		{
			WARNING("ComputeShader surface is nullptr!");
		}
	}

	//void ComputeShader::draw_line(const vec2& p1, const vec2& p2, const Color& color)
	//{
	//	if (_surface != nullptr)
	//	{
	//		ivec2 p1i = { (p1.x + 1.0f) * (width() / 2), (p1.y + 1.0f) * (height() / 2) };
	//		ivec2 p2i = { (p2.x + 1.0f) * (width() / 2), (p2.y + 1.0f) * (height() / 2) };

	//		Color8 color8 = Color8(color);

	//		rasterize_line(p1i, p2i, [&](const LineFragmentData& fragment)
	//		{
	//			*fragment.pixel = color8;
	//		});

	//		update_texture();
	//	}
	//	else
	//	{
	//		WARNING("ComputeShader surface is nullptr!");
	//	}
	//}

	void ComputeShader::draw_line(const DrawLineParams& params)
	{
		if (_surface != nullptr)
		{
			Polygon polygon;

			vec2 normal = normalize(params.p2 - params.p1)/* * MAX(params.thickness1, params.thickness2)*/; // TODO: fix this
			vec2 perpendicular = vec2(-normal.y, normal.x);

			vec2 p1a = params.p1 + (-normal * params.thickness1 - perpendicular * params.thickness1);
			vec2 p1b = params.p1 + (-normal * params.thickness1 + perpendicular * params.thickness1);

			vec2 p2a = params.p2 + (normal * params.thickness2 + perpendicular * params.thickness2);
			vec2 p2b = params.p2 + (normal * params.thickness2 - perpendicular * params.thickness2);

			polygon.push_back(p1a);
			polygon.push_back(p1b);
			polygon.push_back(p2a);
			polygon.push_back(p2b);

			rasterize_polygon(polygon, [&](auto& fragment)
			{
				if (fragment.pixel != nullptr)
				{
					auto dist = distance_to_line(params.p1, params.p2, internal_to_global(fragment.coordinate));
					real thickness = lerp(params.thickness1, params.thickness2, dist.t);

					if (thickness == 0.0f) // Don't divide by 0.
					{
						return;
					}

					dist.distance = CLAMP(dist.distance / thickness, 0.0f, 1.0f);
					dist.distance = 1.0f - exp(dist.distance * params.exponent) / exp(params.exponent);
					Color color = Color::lerp(params.color1, params.color2, dist.t);
					Color8 color8 = Color8(color);

					if (dist.distance < 1.0f)
					{
						fragment.pixel->a = MAX(fragment.pixel->a, (u8)(dist.distance * 255.0f));

						fragment.pixel->r = color8.r; // TODO: may want to blend colors
						fragment.pixel->g = color8.g;
						fragment.pixel->b = color8.b;
					}
				}
			});

			update_texture();
		}
		else
		{
			WARNING("ComputeShader surface is nullptr!");
		}
	}

	void ComputeShader::draw_pixel(const vec2& point, const Color& color)
	{
		if (_surface != nullptr)
		{
			ivec2 pointi = { (point.x + 1.0f) * (width() / 2), (point.y + 1.0f) * (height() / 2) };

			if (pointi.x < 0 || pointi.x > width() || pointi.y < 0 || pointi.y > height())
			{
				return;
			}

			Color8 color8 = Color8(color);

			Color8* pixels = _surface->pixels();
			pixels[pointi.x + pointi.y * _surface->width()] = color8;

			update_texture();
		}
		else
		{
			WARNING("ComputeShader surface is nullptr!");
		}
	}

	s32 ComputeShader::width() const
	{
		if (_surface != nullptr)
		{
			return _surface->width();
		}

		WARNING("ComputeShader surface is nullptr!");

		return -1;
	}

	s32 ComputeShader::height() const
	{
		if (_surface != nullptr)
		{
			return _surface->height();
		}
	
		WARNING("ComputeShader surface is nullptr!");

		return -1;
	}

	Color8* ComputeShader::pixels() const
	{
		if (_surface != nullptr)
		{
			return _surface->pixels();
		}

		WARNING("ComputeShader surface is nullptr!");

		return nullptr;
	}

	u8 ComputeShader::is_internal_coordinate_in_bounds(const ivec2& coordinate) const
	{
		return coordinate.x >= 0 && coordinate.x < width() && coordinate.y >= 0 && coordinate.y < height();
	}

	const Color8& ComputeShader::pixel(ivec2 coordinate) const
	{
		if (_surface != nullptr)
		{
			return _surface->pixels()[coordinate.x + coordinate.y * _surface->width()];
		}

		WARNING("ComputeShader surface is nullptr!");

		static Color8 placeholder = { 0, 0, 0, 0 };
		return placeholder;
	}

	vec2 ComputeShader::global_to_local(const vec2& global_coordinate) const // TODO: handle rotation
	{
		real min_x = INFINITY;
		real max_x = -INFINITY;
		real min_y = INFINITY;
		real max_y = -INFINITY;

		for (s32 i = 0; i < 6; i++)
		{
			auto& vertex = _quad.vertices[i];
			min_x = MIN(vertex.point.x, min_x);
			max_x = MAX(vertex.point.x, max_x);
			min_y = MIN(vertex.point.y, min_y);
			max_y = MAX(vertex.point.y, max_y);
		}

		vec2 result;
		result.x = ((global_coordinate.x - min_x) / abs(max_x - min_x)) * 2.0f - 1.0f;
		result.y = ((global_coordinate.y - min_y) / abs(max_y - min_y)) * 2.0f - 1.0f;

		return result;
	}

	ivec2 ComputeShader::local_to_internal(const vec2& local_coordinate) const
	{
		return
		{
			(local_coordinate.x + 1.0f) * (width() / 2),
			(local_coordinate.y + 1.0f)* (height() / 2)
		};
	}

	vec2 ComputeShader::internal_to_global(const ivec2& internal_coordinate) const
	{
		vec2 t;
		t.x = (real)internal_coordinate.x / width();
		t.y = (real)internal_coordinate.y / height();

		real min_x = INFINITY; // TODO: this is slow.
		real max_x = -INFINITY;
		real min_y = INFINITY;
		real max_y = -INFINITY;

		for (s32 i = 0; i < 6; i++)
		{
			auto& vertex = _quad.vertices[i];
			min_x = MIN(vertex.point.x, min_x);
			max_x = MAX(vertex.point.x, max_x);
			min_y = MIN(vertex.point.y, min_y);
			max_y = MAX(vertex.point.y, max_y);
		}

		vec2 global;
		global.x = min_x + (max_x - min_x) * t.x;
		global.y = min_y + (max_y - min_y) * t.y;

		return global;
	}

	void ComputeShader::update_texture() const // TODO: add update texture function to ComputeShader.
	{
		_isPendingTextureUpdate = true;
	}

	void ComputeShader::on_render()
	{
		if (_surface != nullptr && _texture != nullptr)
		{
			if (_isPendingTextureUpdate)
			{
#ifdef NIGHT_ENABLE_LOGGING
				s32 mask = debug::_Log::_debug_logging_mask;
				debug::_Log::_debug_logging_mask = DEBUG_LOG_MASK_OFF;
#endif

				utility::window().destroy_texture(_window_id);
				_texture = utility::window().create_texture(_window_id, { .width = _surface->width(), .height = _surface->height(), .surface = _surface });

#ifdef NIGHT_ENABLE_LOGGING
				debug::_Log::_debug_logging_mask = mask;
#endif

				ASSERT(_texture != nullptr);
				_isPendingTextureUpdate = false;
			}

			utility::renderer().draw_quad(_quad, _texture);
		}
	}

	vector<vector<ivec2>> ComputeShader::contours() const
	{
		vector<u8> discovered(width() * height(), false);

		vector<vector<ivec2>> result;

		for (s32 y = 0; y < height(); y++)
		{
			for (s32 x = 0; x < width(); x++)
			{
				ivec2 point = { x, y };

				if (discovered[point.x + point.y * width()] || pixel(point).a != 0)
				{
					continue;
				}

				// we are in a clear pixel, evaluate the fill area.
				vector<u8> visited(width() * height(), false);
				vector<ivec2> contour;
				vector<ivec2> open_set;
				open_set.reserve(width() * height());
				open_set.emplace_back(point);

				auto fn = [&](const ivec2 point)
				{
					if (point.x >= 0 && point.x < width() && point.y >= 0 && point.y < height())
					{
						if ((pixel({ point.x , point.y }).a != 0))
						{
							if (!visited[point.x + point.y * width()])
							{
								visited[point.x + point.y * width()] = true;
								open_set.emplace_back(point);
							}
						}
						else
						{
							if (!discovered[point.x + point.y * width()])
							{
								discovered[point.x + point.y * width()] = true;
								open_set.emplace_back(point);
							}
						}
					}
				};


				s32 iterations = 0;

				while (!open_set.empty())
				{
					ivec2 back = open_set.back();
					open_set.pop_back();

					if (pixel({ back.x , back.y }).a != 0)
					{
						contour.push_back({ back.x, back.y });
						continue;
					}

					fn({ back.x - 1, back.y });
					fn({ back.x + 1, back.y });
					fn({ back.x, back.y - 1 });
					fn({ back.x, back.y + 1 });
					iterations++;
				}

				TRACE(iterations);

				result.push_back(contour); // TODO: this probably copies.
			}
		}

		return result;
	}

	//static vector<ivec2> _make_convex_hull(const vector<ivec2>& points) // TODO: fix crash
	//{
	//	if (points.size() < 3)
	//	{
	//		WARNING("Not enough points to make convex hull.");
	//		return {};
	//	}
	//
	//	vector<ivec2> result;
	//
	//	s32 left_most = 0;
	//
	//	for (s32 i = 1; i < points.size(); i++)
	//	{
	//		if (points[i].x < points[left_most].x)
	//		{
	//			left_most = i;
	//		}
	//	}
	//
	//	s32 point_on_hull = left_most;
	//	s32 current;
	//
	//	do
	//	{
	//		result.push_back(points[point_on_hull]);
	//
	//		current = (point_on_hull + 1) % points.size();
	//		for (s32 next = 0; next < points.size(); next++)
	//		{
	//			if (orientation(points[point_on_hull], points[next], points[current]) == EOrientation::CounterClockwise)
	//			{
	//				current = next;
	//			}
	//		}
	//
	//		point_on_hull = current;
	//	} while (point_on_hull != left_most);
	//
	//	return result;
	//}
	//
	//static vector<ivec2> _approximate_convex_to_poly_count(const vector<ivec2>& points, s32 poly_count)
	//{
	//	if (points.size() < poly_count)
	//	{
	//		return points;
	//	}
	//
	//	vector<ivec2> result = points;
	//	pair<real, s32> min_dist;
	//
	//	while (result.size() > poly_count)
	//	{
	//		min_dist = { INFINITY, -1 };
	//
	//		for (s32 i = 0; i < result.size(); i++)
	//		{
	//			const vec2& line_a = result[i];
	//			const vec2& line_b = result[(i + 2) % result.size()];
	//			const vec2& point = result[(i + 1) % result.size()];
	//			real dist = perpendicular_distance(line_a, line_b, point);
	//
	//			if (dist < min_dist.first)
	//			{
	//				min_dist = { dist, (i + 1) % (s32)result.size() };
	//			}
	//		}
	//
	//		result.erase(result.begin() + min_dist.second);
	//
	//	}
	//
	//	return result;
	//}
	//
	//vector<ivec2> ComputeShader::convex(s32 poly_count) const
	//{
	//	if (_surface != nullptr)
	//	{
	//		vector<ivec2> filled_pixels;
	//		Color8* pixels = _surface->pixels();
	//
	//		for (s32 y = 0; y < height(); y++)
	//		{
	//			for (s32 x = 0; x < width(); x++)
	//			{
	//				Color8& pixel = pixels[x + y * width()];
	//				if (pixel.a != 0)
	//				{
	//					filled_pixels.push_back({ x, y });
	//				}
	//			}
	//		}
	//
	//		return _approximate_convex_to_poly_count(_make_convex_hull(filled_pixels), poly_count);
	//	}
	//
	//	return {};
	//}

	void ComputeShader::rasterize_line(const ivec2& p1, const ivec2& p2, function<void(const LineFragmentData&)> fn) const
	{
		if (_surface == nullptr)
		{
			WARNING("Surface is nullptr.");
			return;
		}

		Color8* pixels = this->pixels(); // TODO: cull pixels outside range.
		s32 width = this->width();
		s32 height = this->height();

		s32 dx = p2.x - p1.x;
		s32 dy = p2.y - p1.y;

		s32 d_long = abs(dx);
		s32 d_short = abs(dy);

		ivec2 offset_long = dx > 0 ? ivec2{ 1, 0 } : ivec2{ -1, 0 };
		ivec2 offset_short = dy > 0 ? ivec2{ 0, 1 } : ivec2{ 0, -1 };

		if (d_long < d_short)
		{
			SWAP(d_long, d_short);
			SWAP(offset_short, offset_long);
		}

		ivec2 current = p1;

		s32 error = (d_short * 2) - d_long;
		for (s32 i = 0; i < d_long; i++)
		{
			u8 in_bounds = is_internal_coordinate_in_bounds(current);
			s32 index = current.x + current.y * width;
			real t = (real)i / (real)(d_long - 1);

			//TRACE(t);

			fn(
				{
				.pixel = in_bounds ? &pixels[index] : nullptr,
				.coordinate = current,
				.t = t,
				}
			); // TODO: impl t

			if (error >= 0)
			{
				current += offset_long + offset_short;
				error += (d_short - d_long) * 2;
			}
			else
			{
				current += offset_long;
				error += d_short * 2;
			}
		}
	}

	void ComputeShader::rasterize_line(const vec2& p1, const vec2& p2, function<void(const LineFragmentData&)> fn) const
	{
		ivec2 p1i = local_to_internal(global_to_local(p1));
		ivec2 p2i = local_to_internal(global_to_local(p2));

		rasterize_line(p1i, p2i, fn);

		update_texture();
	}

	void ComputeShader::rasterize_polygon(const Polygon& polygon, function<void(const PolygonFragmentData&)> fn) const
	{
		if (_surface == nullptr)
		{
			WARNING("Surface is nullptr.");
			return;
		}

		struct bounds
		{
			s32 min{ INT_MAX }; // TODO: add S32_MAX and S32_MIN
			s32 max{ INT_MIN };
		};

		s32 width = this->width(); // TODO: add width and height to struct
		s32 height = this->height();

		bounds x_bounds;
		bounds y_bounds;

		vector<ivec2> points;

		for (s32 i = 0; i < polygon.points().size(); i++)
		{
			auto& point = points.emplace_back(local_to_internal(global_to_local(polygon.points()[i])));

			if (point.x < x_bounds.min)
			{
				x_bounds.min = point.x;
			}

			if (point.x > x_bounds.max)
			{
				x_bounds.max = point.x;
			}

			if (point.y < y_bounds.min)
			{
				y_bounds.min = point.y;
			}

			if (point.y > y_bounds.max)
			{
				y_bounds.max = point.y;
			}
		}

		vector<bounds> h_bounds(abs(y_bounds.max - y_bounds.min + 1));
		vector<bounds> v_bounds(abs(x_bounds.max - x_bounds.min + 1));

		for (s32 i = 0; i < points.size(); i++)
		{
			ivec2& p1 = points[i];
			ivec2& p2 = points[(i + 1) % points.size()];

			rasterize_line(p1, p2, [&](const LineFragmentData& fragment)
			{
				if (fragment.pixel == nullptr)
				{
					return; // TODO: figure this out.
				}

				auto& h = h_bounds[fragment.coordinate.y - y_bounds.min];
				auto& v = v_bounds[fragment.coordinate.x - x_bounds.min];

				if (fragment.coordinate.x < h.min)
				{
					h.min = fragment.coordinate.x;
				}

				if (fragment.coordinate.x > h.max)
				{
					h.max = fragment.coordinate.x;
				}

				if (fragment.coordinate.y < v.min)
				{
					v.min = fragment.coordinate.y;
				}

				if (fragment.coordinate.y > v.max)
				{
					v.max = fragment.coordinate.y;
				}
			});
		}

		s32 max_distance = INT_MIN;

		{
			for (s32 i = 0; i < h_bounds.size(); i++)
			{
				if (h_bounds[i].max - h_bounds[i].min > max_distance)
				{
					max_distance = h_bounds[i].max - h_bounds[i].min;
				}
			}

			for (s32 i = 0; i < v_bounds.size(); i++)
			{
				if (v_bounds[i].max - v_bounds[i].min > max_distance)
				{
					max_distance = v_bounds[i].max - v_bounds[i].min;
				}
			}

			max_distance /= 2;
		}


		for (s32 i = 0; i < h_bounds.size(); i++)
		{
			auto xbounds = h_bounds[i];
			s32 y = y_bounds.min + i;
			for (s32 x = xbounds.min; x < xbounds.max; x++)
			{
				ivec2 coordinate = { x, y };
			
				real tx = (real)(coordinate.x - xbounds.min) / (real)(xbounds.max - xbounds.min - 1);

				if (std::_Is_nan(tx)) // TODO: figure this out
				{
					tx = 0.0f;
				}

				auto& ybounds = v_bounds[x - x_bounds.min];

				real ty = (real)(coordinate.y - ybounds.min) / (real)(ybounds.max - ybounds.min - 1);

				if (std::_Is_nan(ty))
				{
					ty = 0.0f;
				}

				s32 xdist = MIN(abs(xbounds.min - x), abs(xbounds.max - x));
				s32 ydist = MIN(abs(ybounds.min - y), abs(ybounds.max - y));

				PolygonFragmentData fragment =
				{
					.pixel = &pixels()[coordinate.x + coordinate.y * width],
					.coordinate = coordinate,
					.tx = tx,
					.ty = ty,
					.distance = MIN(xdist, ydist),
					.max_distance = max_distance
				};

				fn(fragment);
			}
		}

		update_texture();
	}

	void ComputeShader::rasterize_polygon(const Polygon3D& polygon, function<void(const Polygon3DFragmentData&)> fn) const
	{
		if (_surface == nullptr)
		{
			WARNING("Surface is nullptr.");
			return;
		}

		struct bounds
		{
			s32 min{ INT_MAX }; // TODO: add S32_MAX and S32_MIN
			s32 max{ INT_MIN };
			real min_depth{ INFINITY };
			real max_depth{ -INFINITY };
		};

		s32 width = this->width(); // TODO: add width and height to struct
		s32 height = this->height();

		bounds x_bounds;
		bounds y_bounds;

		vector<vec4> projected_points;
		vector<ivec2> points;

		for (s32 i = 0; i < polygon.points().size(); i++)
		{
			auto& point3d = polygon.points()[i];
			auto& proj = projected_points.emplace_back(Renderer3D::project_point_to_view_plane(point3d));
			auto& point = points.emplace_back(local_to_internal(global_to_local(proj)));

			if (point.x < x_bounds.min)
			{
				x_bounds.min = point.x;
			}

			if (point.x > x_bounds.max)
			{
				x_bounds.max = point.x;
			}

			if (point.y < y_bounds.min)
			{
				y_bounds.min = point.y;
			}

			if (point.y > y_bounds.max)
			{
				y_bounds.max = point.y;
			}
		}

		vector<bounds> h_bounds(abs(y_bounds.max - y_bounds.min + 1));
		vector<bounds> v_bounds(abs(x_bounds.max - x_bounds.min + 1));

		for (s32 i = 0; i < points.size(); i++)
		{
			ivec2& p1 = points[i];
			ivec2& p2 = points[(i + 1) % points.size()];

			auto& p1_proj = projected_points[i];
			auto& p2_proj = projected_points[(i + 1) % points.size()];

			rasterize_line(p1, p2, [&](const LineFragmentData& fragment)
			{
				if (fragment.pixel == nullptr)
				{
					return; // TODO: figure this out.
				}

				auto& h = h_bounds[fragment.coordinate.y - y_bounds.min];
				auto& v = v_bounds[fragment.coordinate.x - x_bounds.min];
				real d = lerp(p1_proj.z, p2_proj.z, fragment.t);

				if (fragment.coordinate.x < h.min)
				{
					h.min = fragment.coordinate.x;
					h.min_depth = d;
				}

				if (fragment.coordinate.x > h.max)
				{
					h.max = fragment.coordinate.x;
					h.max_depth = d;
				}

				if (fragment.coordinate.y < v.min)
				{
					v.min = fragment.coordinate.y;
					v.min_depth = d;
				}

				if (fragment.coordinate.y > v.max)
				{
					v.max = fragment.coordinate.y;
					v.max_depth = d;
				}
			});
		}

		s32 max_distance = INT_MIN;

		{
			for (s32 i = 0; i < h_bounds.size(); i++)
			{
				if (h_bounds[i].max - h_bounds[i].min > max_distance)
				{
					max_distance = h_bounds[i].max - h_bounds[i].min;
				}
			}

			for (s32 i = 0; i < v_bounds.size(); i++)
			{
				if (v_bounds[i].max - v_bounds[i].min > max_distance)
				{
					max_distance = v_bounds[i].max - v_bounds[i].min;
				}
			}

			max_distance /= 2;
		}


		for (s32 i = 0; i < h_bounds.size(); i++)
		{
			auto xbounds = h_bounds[i];
			s32 y = y_bounds.min + i;
			for (s32 x = xbounds.min; x < xbounds.max; x++)
			{
				ivec2 coordinate = { x, y };

				real tx = (real)(coordinate.x - xbounds.min) / (real)(xbounds.max - xbounds.min - 1);
				//TRACE(tx);

				if (std::_Is_nan(tx)) // TODO: figure this out
				{
					tx = 0.0f;
				}

				auto& ybounds = v_bounds[x - x_bounds.min];

				real ty = (real)(coordinate.y - ybounds.min) / (real)(ybounds.max - ybounds.min - 1);

				if (std::_Is_nan(ty))
				{
					ty = 0.0f;
				}

				s32 xdist = MIN(abs(xbounds.min - x), abs(xbounds.max - x));
				s32 ydist = MIN(abs(ybounds.min - y), abs(ybounds.max - y));
				real depth = lerp(xbounds.min_depth, xbounds.max_depth, tx);

				Polygon3DFragmentData fragment =
				{
					.pixel = &pixels()[coordinate.x + coordinate.y * width],
					.coordinate = coordinate,
					.tx = tx,
					.ty = ty,
					.distance = MIN(xdist, ydist),
					.max_distance = max_distance,
					.depth = depth
				};

				fn(fragment);
			}
		}

		update_texture();
	}

}