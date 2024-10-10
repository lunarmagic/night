
#include "nightpch.h"
#include "BoxDrawing.h"
#include "math/math.h"
#include "Renderer3D/Renderer3D.h"
#include "algorithm/algorithm.h"
#include "Canvas.h"
#include "math/math.h"
#include "AABB/AABB.h"
#include "BoxDrawing.h"
#include "form/Box.h"

#include "temp.h"

#include <algorithm>

namespace night
{
	// TODO: impl make_rainbow
	Color colors[8] =
	{
		RED * 0.5f,
		CYAN * 0.5f,
		YELLOW * 0.5f,
		PINK * 0.5f,
		PURPLE * 0.5f,
		GREY * 0.5f,
		ORANGE * 0.5f,
		BLUE * 0.5f,
	};

	struct _pixel_distance
	{
		u16 distance;
		u8 buffer;
		u8 alpha;
	};

	//BoxDrawingScoreResult BoxDrawing::submit(const BoxDrawingParams& params)
	BoxDrawing::BoxDrawing(const BoxDrawingParams& params)
	{
		if (params.lines.empty())
		{
			//return { .score = -1 };
			return;
		}

		AABB box_bounds;

		for (s32 i = 0; i < params.lines.size(); i++)
		{
			auto& line = params.lines[i];

			for (s32 j = 0; j < line.size(); j++)
			{
				auto& point = line[j];
				box_bounds.fit_around_point(point);
			}
		}

		box_bounds.left -= box_bounds.width() * (2.0f / params.rasterization_resolution);
		box_bounds.right += box_bounds.width() * (2.0f / params.rasterization_resolution);
		box_bounds.top -= box_bounds.height() * (2.0f / params.rasterization_resolution);
		box_bounds.bottom += box_bounds.height() * (2.0f / params.rasterization_resolution);

		Quad quad(box_bounds);

		// TODO: use macro
		_debug_shader = create<ComputeShader>("Box Drawing Debug Shader", ComputeShaderParams{
			.width = params.rasterization_resolution,
			.height = params.rasterization_resolution,
			.quad = quad
			}
		);
		_debug_shader->fill(0x00);
		//_debug_shader->visibility(ENodeVisibility::Invisible_Tree);

		_compute_shader = create<ComputeShader>("Box Drawing Compute Shader", ComputeShaderParams{
			.width = params.rasterization_resolution,
			.height = params.rasterization_resolution,
			.quad = quad
			}
		);
		_compute_shader->fill(0xFF);
		_compute_shader->visibility(ENodeVisibility::Invisible_Tree);

		ASSERT(_compute_shader != nullptr);

		// create heatmap
		_pixel_distance* pixels = (_pixel_distance*)_compute_shader->pixels();
		const s32 width = _compute_shader->width();
		const s32 height = _compute_shader->height();
		const s32 size = width * height;

		vector<ivec2> neighbor_queue;
		neighbor_queue.reserve(10000);

		vector<ivec2> next;
		next.reserve(10000);

		auto rasterize_line = [&](const LineFragmentData& fragment)
		{
			if (fragment.pixel != nullptr && fragment.pixel->a != 0)
			{
				*fragment.pixel = { 0, 0, 255, 0 };
				neighbor_queue.emplace_back(fragment.coordinate);
			}
		};

		vector<vec2> to_be_convex;

		for (s32 i = 0; i < params.lines.size(); i++)
		{
			auto& line = params.lines[i];

			for (s32 j = 0; j < (s32)line.size() - 1; j++)
			{
				auto& p1 = line[j];
				auto& p2 = line[j + 1];

				_compute_shader->rasterize_line(p1, p2, rasterize_line);

				to_be_convex.push_back(p1); // TODO: remove duplicates.
				to_be_convex.push_back(p2);
			}
		}

		to_be_convex = make_convex_hull(to_be_convex);
		_contour.points(to_be_convex);
		approximate_countour();

		// calculate nearest neighbor for all non-filled pixels.
		u16 distance = 1;

		while (!neighbor_queue.empty()) // TODO: use clipping area of contour to save pixels.
		{
			for (s32 i = 0; i < neighbor_queue.size(); i++)
			{
				auto& pixel_index = neighbor_queue[i];

				ivec2 neighbors[4] =
				{
					pixel_index + ivec2(1, 0),
					pixel_index + ivec2(-1, 0),
					pixel_index + ivec2(0, 1),
					pixel_index + ivec2(0, -1)
				};

				for (s32 j = 0; j < 4; j++)
				{
					auto& neighbor = neighbors[j];
					if(_compute_shader->is_internal_coordinate_in_bounds(neighbor))
					{
						_pixel_distance& pixel = pixels[neighbor.x + neighbor.y * width];
						if (pixel.distance > distance)
						{
							pixel.distance = distance;
							next.emplace_back(neighbor);
						}
					}
				}
			}

			neighbor_queue.swap(next);
			next.clear();
			distance++;
		}

		_should_evaluate_far_corners = params.should_evaluate_far_corners;

		if (params.t_box_approximation_threshhold != -1.0)
		{
			_t_box_approximation_threshhold = params.t_box_approximation_threshhold;
		}

		evaluate_score();

		_area = _contour.area();

		if (_should_save_memory)
		{
			this->~BoxDrawing();
		}

	}

	AABB BoxDrawing::aabb()
	{
		AABB result;

		for (s32 i = 0; i < (s32)_contour.size(); i++)
		{
			auto& point = _contour[i];
			result.fit_around_point(point);
		}

		return result;
	}

	void BoxDrawing::approximate_countour()
	{
		if (_contour.size() < 6)
		{
			return;
		}

		//vector<vec2> test2;
		//for (const auto& i : _contour.points())
		//{
		//	test2.push_back(i * 360.0);
		//}

		//test = detect_lines(_contour.points());

		real arc_len = _contour.arc_length();
		//LOG(arc_len);

		auto& points = _contour.points();

		vector<vec2> result;
		result.push_back(points[0]);

		for (s32 i = 1; i < points.size(); i++) // TODO: remove this.
		{
			auto& point = points[i];

			if (point != result.back())
			{
				result.push_back(point);
			}
		}

		pair<real, s32> min_dist;

		real contour_area = _contour.area();

		while(true)
		{
			min_dist = { INFINITY, -1 };

			for (s32 i = 0; i < result.size(); i++)
			{
				const vec2& line_a = result[i];
				const vec2& line_b = result[(i + 2) % result.size()];
				const vec2& point = result[(i + 1) % result.size()];

				real len = exp(distance(line_a, line_b));
				real p_dist = perpendicular_distance(line_a, line_b, point);
				real dist = p_dist * len;
			
				if (dist < min_dist.first)
				{
					min_dist = { dist, (i + 1) % (s32)result.size() };
				}
			}

			if (result.size() <= 6)
			{
				if (min_dist.first > _t_box_approximation_threshhold * arc_len || result.size() <= 4)
				{
					TRACE("min: ", min_dist.first);
					break;
				}
			}

			ASSERT(min_dist.second != -1);

			result.erase(result.begin() + min_dist.second);
		}

		_contour.points(result);
	}

	// iterate through each pixel of each clip, if the pixel is filled, evaluate how close it is to the borders of the clip
	real BoxDrawing::evaluate_raster_score(const vector<Polygon>& clips)
	{
		if (_compute_shader == nullptr)
		{
			ERROR("Box drawing compute shader is nullptr, name:", name());
			return -1.0f;
		}

		Color8* pixels = _compute_shader->pixels();

		if (!pixels)
		{
			return -1;
		}

		struct poly_sum
		{
			real sum;
			real area;
			s32 count;
		};

		vector<poly_sum> sums;

		for (s32 i = 0; i < clips.size(); i++)
		{
			auto& segment = clips[i];

			sums.push_back({ .sum = 0.0f, .area = segment.area(), .count = 0 });
			poly_sum& sum = sums.back();

			_compute_shader->rasterize_polygon(segment, [&](const PolygonFragmentData& fragment)
			{
				if (fragment.max_distance <= 0)
				{
					return;
				}

				const ivec2& coordinate = fragment.coordinate;

				s32 dist = ((_pixel_distance*)fragment.pixel)->distance;
				dist = std::clamp(dist, 0, fragment.max_distance * 2);

				real t1 = (real)fragment.distance / (real)(fragment.max_distance * 2);
				real t2 = (real)dist / (real)(fragment.max_distance * 2);

				real t = exp((1.0f - abs(t1 - t2)) * 16) / exp(16.0f);

				sum.sum += t;
				sum.count++;

				if (_show_debug_info) // TODO: hide this stuff behind a macro so it doesnt compile with dist build
				{
					Color8* debug_pixels = _debug_shader->pixels();

					Color color = colors[i % 8] * 0.25f;
					color.a *= t;
					debug_pixels[coordinate.x + coordinate.y * _debug_shader->width()] = Color8(color);
				}
			});

			if (sum.count == 0)
			{
				sums.pop_back();
			}
		}

		if (!sums.empty())
		{
			real raw_result = 0.0f;
			real weighted_result = 0.0f;
			real areas = 0.0f;
		
			for (s32 i = 0; i < sums.size(); i++)
			{
				auto& sum = sums[i];
				raw_result += (sum.sum / sum.count);
				weighted_result += (sum.sum / sum.count) * sum.area;
				areas += sum.area;
			}

			weighted_result /= areas;
			raw_result /= sums.size();

			real result = lerp(raw_result, weighted_result, _rasterization_clip_area_weight);
			return result;
		}

		return -1.0;
	}

	BoxDrawing::BoxDrawingCanidate BoxDrawing::evaluate_i_box_canidate(s32 index)
	{
		BoxDrawingCanidate result = {};

		// long vp
		auto& la1 = _contour[index + 0];
		auto& la2 = _contour[index + 1];
		auto& lb1 = _contour[index + 3];
		auto& lb2 = _contour[index + 4];
		vec2 la = normalize(la2 - la1);
		vec2 lb = normalize(lb2 - lb1);
		vec2 vpl = project(la1, la, lb1, lb).contact;

		// short vp 1, connects to la
		auto& s1a1 = _contour[index + 1];
		auto& s1a2 = _contour[index + 2];
		auto& s1b1 = _contour[index + 5];
		auto& s1b2 = _contour[index + 6];
		vec2 s1a = normalize(s1a2 - s1a1);
		vec2 s1b = normalize(s1b2 - s1b1);

		// short vp 2, connects to lb
		auto& s2a1 = _contour[index + 4];
		auto& s2a2 = _contour[index + 5];
		auto& s2b1 = _contour[index + 8];
		auto& s2b2 = _contour[index + 9];
		vec2 s2a = normalize(s2a2 - s2a1);
		vec2 s2b = normalize(s2b2 - s2b1);

		// short vps to find far corner
		vec2 vps1 = project(s1a1, s1a, s1b1, s1b).contact;
		vec2 vps2 = project(s2a1, s2a, s2b1, s2b).contact;

		// connect vp 1 to opposing long vp line.
		vec2 vps1lb1 = normalize(lb1 - vps1);
		vec2 vps1lb2 = normalize(lb2 - vps1);

		// connect vp 2 to opposing long vp line.
		vec2 vps2la1 = normalize(la1 - vps2);
		vec2 vps2la2 = normalize(la2 - vps2);

		// form the far corner line.
		vec2 fc1 = project(vps1, vps1lb1, vps2, vps2la2).contact;
		vec2 fc2 = project(vps2, vps2la1, vps1, vps1lb2).contact;
		vec2 fc = normalize(fc2 - fc1);

		// form the near corner line
		auto& nc1 = _contour[index + 2];
		auto& nc2 = _contour[index + 5];
		vec2 nc = normalize(nc2 - nc1);

		real parallelity_score = dot(normalize(vpl - nc1), nc) * dot(normalize(vpl - fc1), fc); // TODO: maybe add more to this.

		vector<Polygon> clips;

		if (!_should_evaluate_far_corners)
		{
			Polygon h1;
			h1.push_back(la1);
			h1.push_back(la2);
			h1.push_back(s1a2);
			h1.push_back(s1b1);

			Polygon h2;
			h2.push_back(lb1);
			h2.push_back(lb2);
			h2.push_back(s2a2);
			h2.push_back(s2b1);

			clips.push_back(h1);
			clips.push_back(h2);
		}
		else
		{
			// clip _contour into segments divided by vanishing points.
			Polygon segments[4];

			segments[0].push_back(la1);
			segments[0].push_back(fc2);
			segments[0].push_back(lb2);
			segments[0].push_back(nc2);

			segments[1].push_back(la2);
			segments[1].push_back(nc1);
			segments[1].push_back(lb1);
			segments[1].push_back(fc1);

			segments[2].push_back(la1);
			segments[2].push_back(la2);
			segments[2].push_back(fc1);
			segments[2].push_back(fc2);

			segments[3].push_back(fc2);
			segments[3].push_back(fc1);
			segments[3].push_back(lb1);
			segments[3].push_back(lb2);

			s32 count = 0;
			for (s32 j = 0; j < 4; j++)
			{
				auto clip = Polygon::clip(segments[j], nc1, nc);

				if (clip.result)
				{
					clip.a.color = colors[count % 8];
					clip.b.color = colors[(count + 1) % 8];
					count += 2;

					clips.push_back(clip.a);
					clips.push_back(clip.b);
				}
				else
				{
					clips.push_back(segments[j]);
				}
			}
		}

		real raster_score = evaluate_raster_score(clips);

		result.raster_score = raster_score;
		result.i_box_probability = parallelity_score;
		//result.vanishing_points[0] = vpl;
		//result.vanishing_points[1] = vps1;
		//result.vanishing_points[2] = vps2;
		result.vps[0] = vpl;
		result.vps[1] = vps1;
		result.vps[2] = vps2;
		result.clips = clips;
		result.type = EBoxDrawingType::I_BOX;
		result.index = index;

		//result.max_angle = MAX(angle_clockwise(vps1 - vpl, vps2 - vps1), angle_clockwise(vps2 - vps1, vpl - vps2));
		//result.max_angle = MAX(result.max_angle, angle_clockwise(vps2 - vps1, vpl - vps2));

		result.orthocenter = calculate_orthocenter(vpl, vps1, vps2);

		real v = distance(vpl, result.orthocenter);
		real w = perpendicular_distance(vps1, vps2, result.orthocenter);
		result.view_angle = sqrt(v * w);

		result.canidate_probability = result.raster_score * result.i_box_probability;

		//result.vps[0].global_par = abs(dot(normalize(la2 - la1), normalize(lb2 - lb1)));
		//result.vps[1].global_par = abs(dot(normalize(s1a2 - s1a1), normalize(s1b2 - s1b1)));
		//result.vps[2].global_par = abs(dot(normalize(s2a2 - s2a1), normalize(s2b2 - s2b1)));

		return result;
	}

	// IDEA: maybe we can determine y probability by which counter vp a givin corner is most parallel with.
	BoxDrawing::BoxDrawingCanidate BoxDrawing::evaluate_y_box_canidate()
	{
		BoxDrawingCanidate result = {};

		struct vp_data
		{
			s32 index;
			vec2 vp;
			vec2 dir_near;
			vec2 dir_far;

			s32 n;
			s32 f;
			s32 polarity;
			real parallelity;
		};

		auto evaluate_vp = [&](s32 index, u32 polarity) -> vp_data
		{
			const auto& la1 = _contour[0 + index];
			const auto& la2 = _contour[1 + index];
			const auto& lb1 = _contour[3 + index];
			const auto& lb2 = _contour[4 + index];
			auto r = project(la1, la2 - la1, lb1, lb2 - lb1);
			vec2 vp = r.contact;

			s32 n = 2 + index;
			s32 f = 5 + index;
			s32 _polarity = 0;

			if (polarity == 0)
			{
				if (r.t1 < 0.0f)
				{
					SWAP(n, f);
					_polarity = -1;
				}
				else
				{
					_polarity = 1;
				}
			}
			else if (polarity == 1)
			{
				SWAP(n, f);
				_polarity = -1;
			}
			else
			{
				_polarity = 1;
			}

			vec2 dir_near = normalize(vp - _contour[n]);
			vec2 dir_far = normalize(vp - _contour[f]);

			return
			{
				.index = index,
				.vp = vp,
				.dir_near = dir_near,
				.dir_far = dir_far,
				.n = n,
				.f = f,
				.polarity = _polarity,
				.parallelity = abs(dot(normalize(la2 - la1), normalize(lb2 - lb1)))
			};
		};

		vp_data vps[3];

		vps[0] = evaluate_vp(0, 0);
		vps[1] = evaluate_vp(1, 0);
		vps[2] = evaluate_vp(2, 0);

		if (vps[0].polarity == 1) // TODO: find a better way
		{
			if (vps[2].polarity == 1)
			{
				if (vps[1].polarity == 1)
				{
					vps[1] = evaluate_vp(1, vps[1].polarity);
				}
			}
			else if (vps[2].polarity == -1)
			{
				if (vps[1].polarity == 1)
				{
					vps[0] = evaluate_vp(0, vps[0].polarity);
				}
				else if (vps[1].polarity == -1)
				{
					vps[2] = evaluate_vp(2, vps[2].polarity);
				}
			}
		}
		else if(vps[0].polarity == -1)
		{
			if (vps[2].polarity == -1)
			{
				if (vps[1].polarity == -1)
				{
					vps[1] = evaluate_vp(1, vps[1].polarity);
				}
			}
			else if (vps[2].polarity == 1)
			{
				if (vps[1].polarity == -1)
				{
					vps[0] = evaluate_vp(0, vps[0].polarity);
				}
				else if (vps[1].polarity == 1)
				{
					vps[2] = evaluate_vp(2, vps[2].polarity);
				}
			}
		}

		vector<Polygon> clips;

		auto rn01 = project(vps[0].vp, vps[0].dir_near, vps[1].vp, vps[1].dir_near);
		auto rn12 = project(vps[1].vp, vps[1].dir_near, vps[2].vp, vps[2].dir_near);
		auto rn02 = project(vps[0].vp, vps[0].dir_near, vps[2].vp, vps[2].dir_near);

		Polygon clips_near[4];
	
		if (abs((real)rn01.t1) < abs((real)rn02.t1))
		{
			// always order them clockwise.
			if (rn01.is_valid)
			{
				clips_near[0].push_back(_contour[vps[0].n - 1]); // far corner goes in first.
				clips_near[0].push_back(_contour[vps[0].n]);
				clips_near[0].push_back(rn01.contact);
				clips_near[0].push_back(_contour[vps[0].n - 2]);
				clips.push_back(clips_near[0]);
			}

			if (rn12.is_valid)
			{
				clips_near[1].push_back(_contour[vps[1].n - 1]);
				clips_near[1].push_back(_contour[vps[1].n]);
				clips_near[1].push_back(rn12.contact);
				clips_near[1].push_back(_contour[vps[1].n - 2]);
				clips.push_back(clips_near[1]);
			}

			if (rn02.is_valid)
			{
				clips_near[2].push_back(_contour[vps[2].n - 1]);
				clips_near[2].push_back(_contour[vps[2].n]);
				clips_near[2].push_back(rn02.contact);
				clips_near[2].push_back(_contour[vps[2].n - 2]);
				clips.push_back(clips_near[2]);
			}
		}
		else
		{
			// always order them clockwise.
			if (rn02.is_valid)
			{
				clips_near[0].push_back(_contour[vps[0].n + 1]);
				clips_near[0].push_back(_contour[vps[0].n + 2]);
				clips_near[0].push_back(rn02.contact);
				clips_near[0].push_back(_contour[vps[0].n]);
				clips.push_back(clips_near[0]);
			}
	
			if (rn01.is_valid)
			{
				clips_near[1].push_back(_contour[vps[1].n + 1]);
				clips_near[1].push_back(_contour[vps[1].n + 2]);
				clips_near[1].push_back(rn01.contact);
				clips_near[1].push_back(_contour[vps[1].n]);
				clips.push_back(clips_near[1]);
			}
	
			if (rn12.is_valid)
			{
				clips_near[2].push_back(_contour[vps[2].n + 1]);
				clips_near[2].push_back(_contour[vps[2].n + 2]);
				clips_near[2].push_back(rn12.contact);
				clips_near[2].push_back(_contour[vps[2].n]);
				clips.push_back(clips_near[2]);
			}
		}
	
		// the near-triangle in the middle.
		if (rn01.is_valid && rn12.is_valid && rn02.is_valid)
		{
			clips_near[3].push_back(rn01.contact);
			clips_near[3].push_back(rn12.contact);
			clips_near[3].push_back(rn02.contact);
			clips.push_back(clips_near[3]); // this is the near-triangle.
		}


		//clips.push_back(clips_near[0]);
		//clips.push_back(clips_near[1]);
		//clips.push_back(clips_near[2]);
	

		if (_should_evaluate_far_corners)
		{
			// TODO: fix the bug when the box is too parallel.
			//auto rf01 = project(_contour[vps[0].f], vps[0].dir_far, _contour[vps[1].f], vps[1].dir_far);
			//auto rf12 = project(_contour[vps[1].f], vps[1].dir_far, _contour[vps[2].f], vps[2].dir_far);
			//auto rf02 = project(_contour[vps[0].f], vps[0].dir_far, _contour[vps[2].f], vps[2].dir_far);

			ProjectionResult projections_far[3];
			projections_far[0] = project(_contour[vps[0].f], vps[0].dir_far, _contour[vps[1].f], vps[1].dir_far);
			projections_far[1] = project(_contour[vps[1].f], vps[1].dir_far, _contour[vps[2].f], vps[2].dir_far);
			projections_far[2] = project(_contour[vps[0].f], vps[0].dir_far, _contour[vps[2].f], vps[2].dir_far);

			vec2 dir[3];
			dir[0] = -normalize(projections_far[0].contact - _contour[vps[0].f]);
			dir[1] = -normalize(projections_far[1].contact - _contour[vps[1].f]);
			dir[2] = -normalize(projections_far[2].contact - _contour[vps[2].f]);

			//vec2 triangle[3];

			//triangle[0] = rf01.contact;
			//triangle[1] = rf12.contact;
			//triangle[2] = rf02.contact;

			//Window::get().draw_line(projections_far[0].contact, projections_far[0].contact + dir[0] * 1000.0, RED);
			//Window::get().draw_line(projections_far[1].contact, projections_far[1].contact + dir[1] * 1000.0, BLUE);
			//Window::get().draw_line(projections_far[2].contact, projections_far[2].contact + dir[2] * 1000.0, GREEN);

			vector<Polygon> clips_far;

			for(s32 i = 0; i < clips.size(); i++)
			{
				auto& polygon = clips[i].points();
				Polygon clip_buckets[3];
				clip_buckets[0].color = RED;
				clip_buckets[1].color = CYAN;
				clip_buckets[2].color = GREEN;

				for (s32 j = 0; j < 3; j++)
				{
					if (!projections_far[i].is_valid)
					{
						continue;
					}

					auto& tri = projections_far[j].contact;
					auto& dir_1 = dir[j];
					auto& dir_2 = dir[(j + 1) % 3];

					u8 is_tri_in_polygon = true;

					for(s32 k = 0; k < polygon.size(); k++)
					{
						auto& p1 = polygon[k];
						auto& p2 = polygon[(k + 1) % polygon.size()];

						vec2 d = -normalize(tri - p1);

						if (perp_dot(dir_1, d) * perp_dot(dir_1, dir_2) >= 0.0f && perp_dot(dir_2, d) * perp_dot(dir_2, dir_1) >= 0.0f)
						{
							clip_buckets[j].push_back(p1);
						}

						auto proj_1 = project(tri, dir_1, p1, p2 - p1); // TODO: figure out if i should use an epsilon
						if (proj_1.is_valid && proj_1.t2 > -0.01f && proj_1.t2 < 1.01f && proj_1.t1 > 0.0f)
						{
							clip_buckets[j].push_back(proj_1.contact);
						}

						auto proj_2 = project(tri, dir_2, p1, p2 - p1);
						if (proj_2.is_valid && proj_2.t2 > -0.01f && proj_2.t2 < 1.01f && proj_2.t1 > 0.0f)
						{
							clip_buckets[j].push_back(proj_2.contact);
						}

						//if (is_tri_in_polygon && perp_dot(normalize(p2 - p1), normalize(tri - p1)) > 0.0f) // TODO: if hit back of plane and not front, tri must be inside
						if(orientation(p1, tri, p2) == EOrientation::CounterClockwise)
						{
							is_tri_in_polygon = false;
						}
					}

					if (is_tri_in_polygon) // TODO: we can probably do this in the loop
					{
						clip_buckets[j].insert(tri);
					}
				}

				for (s32 j = 0; j < 3; j++)
				{
					auto& clip = clip_buckets[j];
					if (!clip.points().empty())
					{
						clips_far.push_back(clip);
					}
				}

				Polygon far_tri; // TODO: intersect far and near triangles.
				far_tri.push_back(projections_far[0].contact);
				far_tri.push_back(projections_far[1].contact);
				far_tri.push_back(projections_far[2].contact);

				clips_far.push_back(far_tri);
			}

			clips = clips_far;
		}

		result.raster_score = evaluate_raster_score(clips);
		result.clips = clips;
		result.type = EBoxDrawingType::Y_BOX;
		result.index = 0;

		//result.max_angle = MAX(angle_clockwise(vps[1].vp - vps[0].vp, vps[2].vp - vps[1].vp), angle_clockwise(vps[2].vp - vps[1].vp, vps[0].vp - vps[2].vp));
		//result.max_angle = MAX(result.max_angle, angle_clockwise(vps[2].vp - vps[1].vp, vps[0].vp - vps[2].vp));

		result.orthocenter = calculate_orthocenter(vps[0].vp, vps[1].vp, vps[2].vp);

		real v = distance(vps[0].vp, result.orthocenter);
		real w = perpendicular_distance(vps[1].vp, vps[2].vp, result.orthocenter);
		result.view_angle = sqrt(v * w);

		result.vps[0] = vps[0].vp;
		result.vps[1] = vps[1].vp;
		result.vps[2] = vps[2].vp;

		result.canidate_probability = result.raster_score;

		//result.vps[0].global_par = vps[0].parallelity;
		//result.vps[1].global_par = vps[1].parallelity;
		//result.vps[2].global_par = vps[2].parallelity;

		//result.parallelity = MAX(MAX(vps[0].parallelity, vps[1].parallelity), vps[2].parallelity); // TODO: get par from triangle.

		return result;
	}

	BoxDrawing::BoxDrawingCanidate BoxDrawing::evaluate_t_box_canidate()
	{

		return BoxDrawingCanidate();
	}

	void BoxDrawing::evaluate_score()
	{
		if (_contour.size() < 4)
		{
			//return { .score = -1.0f };
			TRACE("Box contour is a triangle.");
			return;
		}
		else if (_contour.size() == 4)
		{
			TRACE("SQ Box");
		}
		else if (_contour.size() == 5)
		{
			TRACE("T Box!");
			evaluate_t_box_canidate();
		}
		else if (_contour.size() == 6)
		{
			BoxDrawingCanidate ci0;
			BoxDrawingCanidate ci1;
			BoxDrawingCanidate ci2;
			BoxDrawingCanidate cy;

			ci0 = evaluate_i_box_canidate(0);
			ci1 = evaluate_i_box_canidate(1);
			ci2 = evaluate_i_box_canidate(2);
			cy = evaluate_y_box_canidate();

			BoxDrawingCanidate* best = nullptr;
			best = ci0.canidate_probability > ci1.canidate_probability ? &ci0 : &ci1;
			best = best->canidate_probability > ci2.canidate_probability ? best : &ci2;
			BoxDrawingCanidate* best_i = best;
			best = best->canidate_probability > cy.canidate_probability ? best : &cy;
		
			if (best->type == EBoxDrawingType::Y_BOX)
			{
				TRACE("Y BOX");
			}
			else if (best->type == EBoxDrawingType::I_BOX)
			{
				TRACE("I_BOX");
			}

			_canidates[0] = *best;
			// TODO: sort _canidates by probability score

			//BoxDrawingScoreResult score;
			//score.score = best->raster_score; // TODO: calculate actual score.

			//_score = score;
			//_vanishing_points[0] = best->vps[0];
			//_vanishing_points[1] = best->vps[1];
			//_vanishing_points[2] = best->vps[2];
			//_type = best->type;
			//_view_angle = best->view_angle;
			//_orthocenter = best->orthocenter;
			//_clips = best->clips;

			TRACE("");
			TRACE("i_box prob: ", best_i->i_box_probability);
			TRACE("y_box prob: ", cy.y_box_probability);
			TRACE("raster_score: ", best->raster_score);
			TRACE("view_angle: ", DEGREES(best->view_angle));
			TRACE("ortho_center: ", best->orthocenter.x, ", ", best->orthocenter.y);
			TRACE("");
		}



		//real bp = MAX(MAX(best_i->vps[0].global_par, best_i->vps[1].global_par), best_i->vps[2].global_par); // TODO: get par from triangle.
		//real yp = MAX(MAX(cy.vps[0].global_par, cy.vps[1].global_par), cy.vps[2].global_par); // TODO: get par from triangle.
		//
		//LOG("y_box view_angle: ", cy.view_angle, ", par: ", yp);
		//LOG("Y: ", cy.vps[0].global_par / (cy.vps[1].global_par * cy.vps[2].global_par));
		//
		//LOG("\n");
		//
		//LOG("i_box view_angle: ", best_i->view_angle, ", par: ", bp);
		//LOG("I: ", best_i->vps[0].global_par / (best_i->vps[1].global_par * best_i->vps[2].global_par));
	
		//LOG(sqrt(cy.vps[0].global_par / cy.vps[1].global_par / cy.vps[2].global_par));
		//if (cy.vps[0].global_par / (cy.vps[1].global_par * cy.vps[2].global_par) > cy.view_angle)
		//{
		//	LOG("YBOX!!!!!!!!!");
		//}

		//auto c = evaluate_y_box_canidate();
		//
		//_score.score = c.raster_score;
		//_clips = c.clips;
		//LOG(_score.score);

		//return _score;
	}

	void BoxDrawing::on_render()
	{
		//auto lines = test.get_lines(15);
		//for (s32 i = 0; i < lines.size(); i++)
		//{
		//	Line& line = lines[i];
		//	//line.p1 /= 360.0f;
		//	//line.p2 /= 360.0f;
		//	vec2 origin = line.p1;
		//	vec2 direction = normalize(line.p2 - line.p1);
		//	Window::get().draw_line(origin, origin + direction * 100.0, RED);
		//	Window::get().draw_line(origin, origin - direction * 100.0, RED);
		//}

		if (_show_debug_info) // TODO: add debug renderer.
		{
			//_debug_shader.render();

			for (const auto& i : _canidates[0].clips)
			{
				i.draw();
			}

			//_compute_shader.render();

			_contour.draw();
		}
	}

}