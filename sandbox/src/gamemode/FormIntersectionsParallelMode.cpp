
#include "nightpch.h"
#include "FormIntersectionsParallelMode.h"
#include "form/Box.h"
#include "math/math.h"
#include "input/InputKey.h"
#include "input/EInput.h"
#include "application/Application.h"
#include "Renderer3D/Renderer3D.h"
//#include "Drawing/Line.h"
#include "form/Intersection.h"
#include "geometry/Quad.h"
#include "Drawing/Canvas.h"
#include "algorithm/algorithm.h"
#include "gjk/gjk.h"

namespace night
{
	FormIntersectionsParallelMode::FormIntersectionsParallelMode(const FormIntersectionsParallelModeParams& params)
	{
		Renderer3D::mode(EPerspectiveMode::Parallel);
		_params = params;

		BIND_INPUT(EKey::DELETE, EInputType::PRESSED, [&]() { reset(); });
		BIND_INPUT(EKey::RETURN, EInputType::PRESSED, [&]() { submit(); });

		depth(500.0f); // debug

		_canvas = create<Canvas>("Canvas", CanvasParams
			{
				.pen_color = _canvasPenColor,
				.compute_shader_params = {.width = params.internal_resolution.x, .height = params.internal_resolution.y }
			}
		);
		_canvas->depth(100.0f);
		_canvas->compute_shader()->depth(100.0f); // TODO: make it so that depth is passed down to children.

		_drawingUDF = create<ComputeShader>("Drawing Unsigned Distance Field", ComputeShaderParams
			{
				.width = params.internal_resolution.x,
				.height = params.internal_resolution.y
			}
		);
		_drawingUDF->depth(0.0f);
		_drawingUDF->visibility(ENodeVisibility::Invisible);

		_intersectionsOfTimeUDF = create<ComputeShader>("Intersections Unsigned Distance Field", ComputeShaderParams
			{
				.width = params.internal_resolution.x,
				.height = params.internal_resolution.y
			}
		);
		_intersectionsOfTimeUDF->depth(0.0f);
		_intersectionsOfTimeUDF->visibility(ENodeVisibility::Invisible);

		// TODO: handle debug renderer.
		_instersectionsDebugView = create<ComputeShader>("debug view", ComputeShaderParams
			{
				.width = params.internal_resolution.x,
				.height = params.internal_resolution.y
			}
		);
		_instersectionsDebugView->depth(1.0f);
		_instersectionsDebugView->visibility(ENodeVisibility::Invisible);

		spawn_forms();
	}

	void FormIntersectionsParallelMode::on_render()
	{
		auto time_elapsed = utility::window().time_elapsed();
		auto& current = _debug_intersections[s32(time_elapsed * 10) % _debug_intersections.size()];
		
		if (_is_submitted)
		{
			for (s32 i = 0; i < _intersections.size(); i++)
			{
				auto& fi = _intersections[i];
				for (s32 j = 0; j < fi.intersections.size(); j++)
				{
					auto& intersection = fi.intersections[j];

					constexpr real range = 3.5f;
					auto [p1, p2] = intersection_of_time((fmod(time_elapsed, range) - range / 2), intersection);

					if (p1 != p2)
					{
						utility::renderer().draw_line(p1, p2, BLUE);
					}

					auto [ap1, ap2] = intersection_of_time(fi.average_toi, intersection);

					if (ap1 == ap2)
					{
						continue;
					}

					utility::renderer().draw_line(ap1, ap2, RED);

					//for (s32 k = 0; k < intersection.depth_buffer.size() - 1; k++)
					//{
					//	if (intersection.depth_buffer[k] == INFINITY)
					//	{
					//		continue;
					//	}
					//
					//	real t1 = (real)k / (real)(intersection.depth_buffer.size() - 1);
					//	real t2 = (real)(k + 1) / (real)(intersection.depth_buffer.size() - 1);
					//	Color color = Color::lerp(GREEN, RED, intersection.depth_buffer[k] * 10.0f);
					//	//color.a -= intersection.depth_buffer[k] * 10.0f;
					//	//color.a = CLAMP(color.a, 0, 1);
					//	utility::renderer().draw_line(ap1 + (ap2 - ap1) * t1, ap1 + (ap2 - ap1) * t2, color);
					//}

					/*auto[ap1, ap2] = intersection_of_time(fi.average_toi, intersection);

					if (ap1 == ap2)
					{
						continue;
					}

					s32 current_bound = 0;
					real lower_t = 0.0f;
					real upper_t = 0.0f;

					for (auto k = intersection.area_coverage.begin(); k != intersection.area_coverage.end(); k++)
					{
						if (current_bound == 0)
						{
							upper_t = (*k).t;
					
							if (upper_t > 0.0f)
							{
								vec2 t1 = ap1 + (ap2 - ap1) * lower_t;
								vec2 t2 = ap1 + (ap2 - ap1) * upper_t;
								utility::renderer().draw_line(t1, t2, RED);
							}

							lower_t = upper_t;
						}
						else
						{
							lower_t = (*k).t;
						}
					
						current_bound += (*k).bound;
					}
					
					if (current_bound == 0 && lower_t < 1.0f)
					{
						vec2 t1 = ap1 + (ap2 - ap1) * lower_t;
						vec2 t2 = ap1 + (ap2 - ap1) * 1.0;
						utility::renderer().draw_line(t1, t2, RED);
					}*/
				}
			}
		}

		//for (s32 i = 0; i < _intersections.size(); i++)
		//{
		//	auto& form = _intersections[i];
		//	for (s32 j = 0; j < form.intersections.size(); j++)
		//	{
		//		auto& intersection = form.intersections[j];
		//		vec4 origin = Renderer3D::project_point_to_view_plane(vec4(intersection.origin, 1));
		//		vec4 normal = Renderer3D::project_point_to_view_plane(vec4(intersection.normal, 1));
		//		utility::renderer().draw_line(origin, origin + normal * 100.0, GREEN);
		//		utility::renderer().draw_line(origin, origin - normal * 100.0, GREEN);
		//		intersection.area.draw();
		//
		//		//constexpr real range = 2.5f;
		//		//auto [p1, p2] = intersection_of_time((fmod(time_elapsed, range) - range / 2), intersection);
		//		//utility::renderer().draw_line(p1, p2, PURPLE);
		//	}
		//}
	}

	// TODO: only check pixels within the intersecting form overlap area, this is needed when there are multiple forms intersecting.
	void FormIntersectionsParallelMode::submit()
	{
		_score = 1.0f;

		//// calculate average toi.
		//for (s32 i = 0; i < _intersections.size(); i++)
		//{
		//	auto& fi = _intersections[i]; // fi is every plane of the forms that are overlapping.
		//	vector<real> tois;
		//
		//	real avg_toi = 0.0f;
		//	s32 toi_count = 0;
		//
		//	for (s32 j = 0; j < fi.intersections.size(); j++)
		//	{
		//		auto& intersection = fi.intersections[j]; // intersection is each plane that are overlapping.
		//		auto& vertices = intersection.area.points();
		//
		//		vec2 avg_point = { 0, 0 };
		//		s32 point_count = 0;
		//
		//		for (s32 k = 0; k < _canvas->lines().size(); k++)
		//		{
		//			auto& lines = _canvas->lines()[k];
		//			for (s32 l = 0; l < lines.size(); l++)
		//			{
		//				auto& point = lines[l];
		//
		//				if (gjk::overlap(&point, 1, vertices.data(), vertices.size()))
		//				{
		//					real toi = time_of_intersection(point, intersection);
		//					tois.push_back(toi);
		//					avg_point += point;
		//					point_count++;
		//				}
		//			}
		//		}
		//
		//		if (point_count > 0)
		//		{
		//			avg_point /= point_count;
		//			avg_toi += time_of_intersection(avg_point, intersection);
		//			toi_count++;
		//		}
		//	}
		//
		//	if (toi_count <= 0)
		//	{
		//		continue;
		//	}
		//
		//	avg_toi /= toi_count;
		//
		//	fi.average_toi = avg_toi; // TODO: remove
		//}
		
		// UDF
		struct _pixel_distance
		{
			u16 distance;
			u8 buffer;
			u8 alpha;
		};

		//_pixel_distance* pixels = (_pixel_distance*)_drawingUDF->pixels();
		//const s32 width = _drawingUDF->width();
		//const s32 height = _drawingUDF->height();
		//const s32 size = width * height;

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

		_drawingUDF->fill(0xFF); // TODO: may not need to do this.

		for (s32 i = 0; i < _canvas->lines().size(); i++)
		{
			auto& line = _canvas->lines()[i];
			for (s32 j = 0; j < line.size() - 1; j++)
			{
				auto& p1 = line[j];
				auto& p2 = line[j + 1];

				if (p1 == p2)
				{
					continue;
				}

				_drawingUDF->rasterize_line(p1, p2, rasterize_line); // TODO: do this as you draw.
			}
		}

		vector<ivec2> drawn_pixels = neighbor_queue;

		// calculate nearest neighbor for all non-filled pixels.
		auto process_udf = [&](const ref<ComputeShader>& udf)
		{
			_pixel_distance* pixels = (_pixel_distance*)udf->pixels();
			const s32 width = udf->width();
			const s32 height = udf->height();
			const s32 size = width * height;

			u16 udf_distance = 1;

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
						if (udf->is_internal_coordinate_in_bounds(neighbor))
						{
							_pixel_distance& pixel = pixels[neighbor.x + neighbor.y * width];
							if (pixel.distance > udf_distance)
							{
								pixel.distance = udf_distance;
								next.emplace_back(neighbor);
							}
						}
					}
				}

				neighbor_queue.swap(next);
				next.clear();
				udf_distance++;
			}
		};

		process_udf(_drawingUDF);

		//// check distances from intersections to nearest drawn pixel.
		//real dist_sum = 0.0f;
		//s32 dist_count = 0;
		//
		//for (s32 i = 0; i < _intersections.size(); i++)
		//{
		//	auto& fi = _intersections[i];
		//	for (s32 j = 0; j < fi.intersections.size(); j++)
		//	{
		//		auto& intersection = fi.intersections[j];
		//		auto [ip1, ip2] = intersection_of_time(fi.average_toi, intersection);
		//
		//		_unsignedDistanceField->rasterize_line(ip1, ip2, [&](auto& fragment)
		//			{
		//				if (fragment.pixel == nullptr)
		//				{
		//					return;
		//				}
		//
		//				_pixel_distance* dist = (_pixel_distance*)fragment.pixel;
		//				dist_sum += (real)dist->distance /*/ abs(intersection.slope)*/; // TODO: add form lines to udf
		//				dist_count++;
		//				//TRACE(dist->distance);
		//			}
		//		);
		//	}
		//}                                                                  
		//
		//if (dist_count >= 0)
		//{
		//	dist_sum /= dist_count;
		//	_score /= 1.0f + (dist_sum / 30.0f);
		//	TRACE("score: ", _score);
		//}

		// UDF QUERY
		struct IOTData
		{
			struct {
				real distance_sum{ 0 };
				s32 count{ 0 };
				real intersection_length{ 0 };
			} data[FORM_INTERSECTION_IOT_ACC];

			//pair<real, s32> distances[iot_acc];
		};

		constexpr real iot_range = 2.0f;
		
		for (s32 i = 0; i < _intersections.size(); i++)
		{
			auto& fi = _intersections[i];
			vector<IOTData> iotd_sum;

			for (s32 j = 0; j < fi.intersections.size(); j++)
			{
				auto& intersection = fi.intersections[j];
				auto& iotd = iotd_sum.emplace_back();
				
				for (s32 k = 0; k < FORM_INTERSECTION_IOT_ACC; k++)
				{
					real t = (real)k / (real)(FORM_INTERSECTION_IOT_ACC - 1);
					auto& data = iotd.data[k];
					auto [ip1, ip2] = intersection_of_time(lerp(-iot_range, iot_range, t), intersection);

					if (ip1 == ip2)
					{
						continue;
					}

					data.intersection_length += distance(ip1, ip2);

					_drawingUDF->rasterize_line(ip1, ip2, [&](auto& fragment)
					{
						if (fragment.pixel == nullptr)
						{
							return;
						}

						_pixel_distance* dist = (_pixel_distance*)fragment.pixel;
						data.distance_sum += dist->distance/* / abs(intersection.slope)*/;
						data.count++;
					}
					);
				}
			}

			real min_dist = INFINITY;
			real approx_toi = INFINITY;

			for (s32 j = 0; j < FORM_INTERSECTION_IOT_ACC; j++)
			{
				real t = (real)j / (real)(FORM_INTERSECTION_IOT_ACC - 1);
				real iot_dist_sum = 0;
				s32 iot_dist_count = 0;

				for (s32 k = 0; k < iotd_sum.size(); k++)
				{
					auto& iotd = iotd_sum[k];
					auto& data = iotd.data[j];

					if (data.count != 0)
					{
						// div by toi_length means bigger intersections give more points.
						iot_dist_sum += (data.distance_sum / data.count)/* / toi_length*/;
						iot_dist_count++;
					}
				}

				if (iot_dist_count == 0)
				{
					continue;
				}

				iot_dist_sum /= iot_dist_count;

				// INVERSE UDF:
				_intersectionsOfTimeUDF->fill(0xFF);
				neighbor_queue.clear();
				next.clear();

				for (s32 k = 0; k < fi.intersections.size(); k++)
				{
					auto& intersection = fi.intersections[k];
					auto [ip1, ip2] = intersection_of_time(lerp(-iot_range, iot_range, t), intersection);

					if (ip1 == ip2)
					{
						continue;
					}

					_intersectionsOfTimeUDF->rasterize_line(ip1, ip2, rasterize_line);
				}

				process_udf(_intersectionsOfTimeUDF);

				// iterate over all drawn pixels and compare them to the udf.
				real inverse_dist_sum = 0;
				s32 inverse_dist_count = 0;
				for (s32 k = 0; k < drawn_pixels.size(); k++)
				{
					auto& coordinate = drawn_pixels[k];

					if (_intersectionsOfTimeUDF->is_internal_coordinate_in_bounds(coordinate))
					{
						_pixel_distance& distance = (_pixel_distance&)_intersectionsOfTimeUDF->pixel(coordinate);
						inverse_dist_sum += (real)distance.distance;
						inverse_dist_count++;
					}
				}

				if (inverse_dist_count > 0)
				{
					inverse_dist_sum /= inverse_dist_count;
				}

				TRACE("inverse_dist_sum: ", inverse_dist_sum);

				// calc approx_toi:
				//real dist_sum = lerp(iot_dist_sum, inverse_dist_sum, 0.5f);
				real dist_sum = iot_dist_sum / (1.0f / inverse_dist_sum);

				if (dist_sum < min_dist)
				{
					min_dist = dist_sum;
					approx_toi = lerp(-iot_range, iot_range, t); // TODO: store toi in data struct
				}
			}

			fi.average_toi = approx_toi;
			fi.distance_to_pixels = min_dist;

			TRACE("toi: ", fi.average_toi, ", dist: ", fi.distance_to_pixels, ", inv: ",
				(1.0 / fi.distance_to_pixels));

		}

		//for (s32 i = 0; i < _intersections.size(); i++)
		//{
		//	auto& fi = _intersections[i];
		//	for (s32 j = 0; j < fi.intersections.size(); j++)
		//	{
		//		auto& intersection = fi.intersections[j];
		//
		//		auto [ip1, ip2] = intersection_of_time(fi.average_toi, intersection);
		//		//intersection.depth_buffer.fill(INFINITY);
		//
		//		if (ip1 == ip2)
		//		{
		//			continue;
		//		}
		//
		//		//for (s32 k = 0; k < _canvas->lines().size(); k++)
		//		//{
		//		//	auto& lines = _canvas->lines()[k];
		//		//	for (s32 l = 0; l < lines.size() - 1; l++)
		//		//	{
		//		//		auto& lp1 = lines[l];
		//		//		auto& lp2 = lines[l + 1];
		//		//
		//		//		auto pp1 = distance_to_plane(ip1, ip2, lp1);
		//		//		auto pp2 = distance_to_plane(ip1, ip2, lp2);
		//		//
		//		//		if ((pp1.t < 0 && pp2.t < 0) || (pp1.t > 1 && pp2.t > 1))
		//		//		{
		//		//			continue; // the line is outside the bounds of the intersection.
		//		//		}
		//		//
		//		//		s32 i1 = CLAMP(s32(pp1.t * intersection.depth_buffer.size()), 0, intersection.depth_buffer.size());
		//		//		s32 i2 = CLAMP(s32(pp2.t * intersection.depth_buffer.size()), 0, intersection.depth_buffer.size());
		//		//
		//		//		if (i1 > i2)
		//		//		{
		//		//			SWAP(i1, i2);
		//		//			SWAP(pp1, pp2);
		//		//		}
		//		//
		//		//		//TRACE(i1, i2);
		//		//
		//		//		for (s32 d = i1; d < i2; d++)
		//		//		{
		//		//			real t = (real)(d - i1) / (real)(i2 - i1);
		//		//			real dist = lerp(pp1.distance, pp2.distance, t);
		//		//			intersection.depth_buffer[d] = MIN(dist, intersection.depth_buffer[d]);
		//		//		}
		//		//
		//		//		//TRACE("dists: ");
		//		//		//TRACE(pp1.distance);
		//		//		//TRACE(pp1.distance);
		//		//		//TRACE("\n");
		//		//	}
		//		//}
		//		//
		//		//TRACE("Intersection Dists:");
		//		//for (s32 i = 0; i < intersection.depth_buffer.size(); i++)
		//		//{
		//		//	TRACE(intersection.depth_buffer[i]);
		//		//}
		//		//TRACE("\n");
		//	}
		//}

		_is_submitted = true;

		//_score = 1.0f;
		//
		//for (s32 i = 0; i < _intersections.size(); i++)
		//{
		//	auto& fi = _intersections[i];
		//	vector<real> tois;
		//
		//	real avg_toi = 0.0f;
		//	s32 toi_count = 0;
		//	
		//	// calc variance from the lines to the intersection.
		//	for (s32 j = 0; j < fi.intersections.size(); j++)
		//	{
		//		auto& intersection = fi.intersections[j];
		//		auto& vertices = intersection.area.points();
		//
		//		vec2 avg_point(0);
		//		s32 point_count = 0;
		//
		//		for (s32 k = 0; k < _canvas->lines().size(); k++)
		//		{
		//			auto& lines = _canvas->lines()[k];
		//			for (s32 l = 0; l < lines.size(); l++)
		//			{
		//				auto& point = lines[l];
		//
		//				if (gjk::overlap(&point, 1, vertices.data(), vertices.size()))
		//				{
		//					real toi = time_of_intersection(point, intersection);
		//					tois.push_back(toi);
		//					avg_point += point;
		//					point_count++;
		//				}
		//			}
		//		}
		//
		//		if (point_count > 0)
		//		{
		//			avg_point /= point_count;
		//			avg_toi += time_of_intersection(avg_point, intersection);
		//			toi_count++;
		//		}
		//	}
		//
		//	if (toi_count <= 0)
		//	{
		//		_score = -1;
		//		return;
		//	}
		//
		//	avg_toi /= toi_count;
		//
		//	fi.average_toi = avg_toi; // TODO: remove
		//
		//	real mean = 0.0f;
		//
		//	for (s32 j = 0; j < tois.size(); j++)
		//	{
		//		mean += tois[j];
		//	}
		//
		//	mean /= tois.size();
		//
		//	real variance = 0.0f;
		//
		//	for (s32 j = 0; j < tois.size(); j++)
		//	{
		//		real v = abs((tois[j] - mean) * (tois[j] - mean));
		//		v *= _intersectionVarianceWeight;
		//		variance += 1.0f + v;
		//	}
		//
		//	variance /= tois.size();
		//	_score /= variance;
		//
		//	// negate points from score if a segment of the avg intersection is missing lines.
		//	for (s32 j = 0; j < fi.intersections.size(); j++)
		//	{
		//		real to_negate = 1.0f;
		//		constexpr real to_negate_weight = 1.0f; // TODO: add weight, exponent, ect. struct members to all score tallying
		//
		//		auto& intersection = fi.intersections[j];
		//		intersection.area_coverage.clear();
		//		auto& vertices = intersection.area.points();
		//
		//		auto [p1, p2] = intersection_of_time(fi.average_toi, intersection);
		//
		//		if (p1 == p2)
		//		{
		//			continue;
		//		}
		//
		//		for (s32 k = 0; k < _canvas->lines().size(); k++)
		//		{
		//			auto& lines = _canvas->lines()[k];
		//			for (s32 l = 0; l < lines.size() - 1; l++)
		//			{
		//				auto& l1 = lines[l];
		//				auto& l2 = lines[l + 1];
		//
		//				if (gjk::overlap(&l1, 2, vertices.data(), vertices.size()))
		//				{
		//					auto [pl1, t_max] = project_point_to_plane(l1, p1, p2 - p1);
		//					auto [pl2, t_min] = project_point_to_plane(l2, p1, p2 - p1);
		//
		//					t_max = CLAMP(t_max, 0, 1);
		//					t_min = CLAMP(t_min, 0, 1);
		//
		//					if (t_min > t_max)
		//					{
		//						SWAP(t_min, t_max);
		//					}
		//
		//					IntersectionCoverageBound min_bound = { .t = t_min, .bound = INTERSECTION_COVERAGE_BOUND_MIN };
		//					IntersectionCoverageBound max_bound = { .t = t_max, .bound = INTERSECTION_COVERAGE_BOUND_MAX };
		//
		//					intersection.area_coverage.insert(min_bound);
		//					intersection.area_coverage.insert(max_bound);
		//				}
		//			}
		//		}
		//
		//		s32 current_bound = 0;
		//		real lower_t = 0.0f;
		//		real upper_t = 0.0f;
		//
		//		for (auto k = intersection.area_coverage.begin(); k != intersection.area_coverage.end(); k++)
		//		{
		//			if (current_bound == 0)
		//			{
		//				upper_t = (*k).t;
		//
		//				if (upper_t > 0.0f)
		//				{
		//					vec2 t1 = p1 + (p2 - p1) * lower_t;
		//					vec2 t2 = p1 + (p2 - p1) * upper_t;
		//					to_negate += length(t2 - t1) * to_negate_weight;
		//				}
		//
		//				lower_t = upper_t;
		//			}
		//			else
		//			{
		//				lower_t = (*k).t;
		//			}
		//
		//			current_bound += (*k).bound;
		//		}
		//
		//		if (current_bound == 0 && lower_t < 1.0f)
		//		{
		//			vec2 t1 = p1 + (p2 - p1) * lower_t;
		//			vec2 t2 = p1 + (p2 - p1) * 1.0;
		//			to_negate += length(t2 - t1) * to_negate_weight;
		//		}
		//
		//		_score /= to_negate;
		//	}
		//}
		//
		//TRACE("score: ", _score);
		//
		//_is_submitted = true;
	}

	void FormIntersectionsParallelMode::reset()
	{
		clear_forms();

		if (_canvas == nullptr)
		{
			ERROR(name(), "'s _canvas is nullptr!");
			return;
		}

		_canvas->clear();
		_drawingUDF->fill(0xFF);
		
		spawn_forms();
	}

	void FormIntersectionsParallelMode::spawn_forms()
	{
		_is_submitted = false;

		// TODO: load forms from file, create editor.

		// Spawn in the forms with variability and density in there positions.
		//for (s32 y = 0; y < _params.density.y; y++)
		//{
		//	for (s32 x = 0; x < _params.density.x; x++)
		//	{
		//		real tx = (real)x / (real)(_params.density.x - 1);
		//		real ty = (real)y / (real)(_params.density.y - 1);
		//
		//		auto& form = _forms.emplace_back(create<Box>("Form #" + to_string(x + y * _params.density.x), BoxParams{
		//			.transform = mat4(1),
		//			.extents = {1, 1, 1},
		//			.color = _wireframeColor
		//			}
		//		));
		//
		//		if (form != nullptr)
		//		{
		//			//form->visibility(ENodeVisibility::Invisible_Tree); // TODO: render forms to wireframe shader with z = line weight / edge depth.
		//
		//			vec3 position = vec3(
		//				-_params.area.x / 2.0f + _params.area.x * tx + (random(_params.variation.x) - _params.variation.x / 2),
		//				-_params.area.y / 2.0f + _params.area.y * ty + (random(_params.variation.y) - _params.variation.y / 2),
		//				0.0f
		//			);
		//
		//			form->translate(position);
		//			form->rotate({ 1.0f, 0.0f, 0.0f }, random(R_PI)); // TODO: make random vector function.
		//			form->rotate({ 0.0f, 1.0f, 0.0f }, random(R_PI));
		//			form->rotate({ 0.0f, 0.0f, 1.0f }, random(R_PI));
		//		}
		//	}
		//}

		auto& form1 = _forms.emplace_back(create<Box>("Form #1", BoxParams{
			.transform = mat4(1),
			.extents = {1, 1, 1},
			.color = _wireframeColor
			}
		));

		if (form1 != nullptr)
		{
			vec3 position = vec3(
				-_params.area.x / 2.0f + _params.area.x * 0.0f + (random(_params.variation.x) - _params.variation.x / 2),
				-_params.area.y / 2.0f + _params.area.y * 0.0f + (random(_params.variation.y) - _params.variation.y / 2),
				0.0f
			);

			form1->translate(position);
			form1->rotate({ 1.0f, 0.0f, 0.0f }, random(R_PI)); // TODO: make random vector function.
			form1->rotate({ 0.0f, 1.0f, 0.0f }, random(R_PI));
			form1->rotate({ 0.0f, 0.0f, 1.0f }, random(R_PI));
			form1->depth(5.0f);
		}

		auto& form2 = _forms.emplace_back(create<Box>("Form #2", BoxParams{
				.transform = mat4(1),
				.extents = {1, 1, 1},
				.color = _wireframeColor
			}
		));

		if (form2 != nullptr)
		{
			vec3 position = vec3(
				-_params.area.x / 2.0f + _params.area.x * 0.5f + (random(_params.variation.x) - _params.variation.x / 2),
				-_params.area.y / 2.0f + _params.area.y * 0.5f + (random(_params.variation.y) - _params.variation.y / 2),
				0.0f
			);

			form2->translate(position);
			form2->rotate({ 1.0f, 0.0f, 0.0f }, random(R_PI)); // TODO: make random vector function.
			form2->rotate({ 0.0f, 1.0f, 0.0f }, random(R_PI));
			form2->rotate({ 0.0f, 0.0f, 1.0f }, random(R_PI));
			form2->depth(5.0f);
		}


		// TODO: use slope to render all possible intersections.
		for (s32 i = 0; i < 25; i++)
		{
			auto& current = _debug_intersections.emplace_back();
			real t = (real)i / 10.0f;
			vec3 a_offset = { 0.0f, 0.0f, lerp(-0.5f, 0.5f, t) };
			vec3 b_offset = { 0.0f, 0.0f, lerp(0.5f, -0.5f, t) };

			for (s32 j = 0; j < _forms.size(); j++)
			{
				for (s32 k = j + 1; k < _forms.size(); k++)
				{
					auto& a = _forms[j];
					auto& b = _forms[k];

					auto intersection = Intersection::intersect_test(a, a_offset, b, b_offset);
					current.push_back(intersection);
				}
			}
		}

		// actual intersections
		for (s32 i = 0; i < _forms.size() - 1; i++)
		{
			for (s32 j = i + 1; j < _forms.size(); j++)
			{
				auto& a = _forms[i];
				auto& b = _forms[j];
		
				auto intersection = intersect(a, b);
				_intersections.push_back( intersection );
			}
		}

		// render debug view.
		for (s32 i = 0; i < _intersections.size(); i++)
		{
			auto& fi = _intersections[i];
			for (s32 j = 0; j < fi.intersections.size(); j++)
			{
				auto& intersection = fi.intersections[j];
				vec2 p1 = Renderer3D::project_point_to_view_plane(vec4(intersection.origin, 1));
				//vec2 p2 = Renderer3D::project_point_to_view_plane(vec4(intersection.origin + intersection.normal, 1));
				vec2 n = normalize(Renderer3D::project_point_to_view_plane(vec4(intersection.normal, 1)));
				//vec2 n = normalize(p2 - p1);

				_instersectionsDebugView->rasterize_polygon(intersection.area, [&](auto& fragment)
				{
					if (fragment.pixel)
					{
						vec2 global = _instersectionsDebugView->internal_to_global(fragment.coordinate);

						vec2 proj = project_point_to_plane(global, p1, n).point;

						real slope = perp_dot(n, proj - global);
						slope /= intersection.slope;

						//real dist = perpendicular_distance(p1, p2, global);
						slope = CLAMP(slope * 16, -1, 1);
						slope += 1;
						slope /= 2;

						*fragment.pixel = Color::lerp(GREEN, RED, slope);
					}
				});
			}
		}
	}

	real sign(vec2 p1, vec2 p2, vec2 p3)
	{
		return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	}

	u8 is_inside(vec2 v1, vec2 v2, vec2 v3, vec2 pt)
	{
		real d1, d2, d3;
		u8 has_neg, has_pos;

		d1 = sign(pt, v1, v2);
		d2 = sign(pt, v2, v3);
		d3 = sign(pt, v3, v1);

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(has_neg && has_pos);
	}

	FormIntersectionsParallelMode::FormIntersections FormIntersectionsParallelMode::intersect(ref<Box> form_a, ref<Box> form_b)
	{
		FormIntersections result;

		result.form_a = form_a;
		result.form_b = form_b;

		auto planes_a = form_a->planes();
		auto planes_b = form_b->planes();

		for (s32 i = 0; i < planes_a.size(); i++)
		{
			for (s32 j = 0; j < planes_b.size(); j++)
			{
				auto& plane_a = planes_a[i];
				auto& plane_b = planes_b[j];

				if (
					Renderer3D::should_cull_face(
						vec4(plane_a.vertices[0], 1),
						vec4(plane_a.vertices[1], 1),
						vec4(plane_a.vertices[2], 1)
					)
					||
					Renderer3D::should_cull_face(
						vec4(plane_b.vertices[0], 1),
						vec4(plane_b.vertices[1], 1),
						vec4(plane_b.vertices[2], 1)
					))
				{
					continue;
				}

				array<vec4, 4> plane_a_proj;
				array<vec4, 4> plane_b_proj;

				for (s32 k = 0; k < 4; k++)
				{
					plane_a_proj[k] = Renderer3D::project_point_to_view_plane(vec4(plane_a.vertices[k], 1));
					plane_b_proj[k] = Renderer3D::project_point_to_view_plane(vec4(plane_b.vertices[k], 1));
				}

				Polygon area;
				area.color = BLUE;

				for (s32 k = 0; k < plane_a_proj.size(); k++)
				{
					auto& ap1 = plane_a_proj[k];
					auto& ap2 = plane_a_proj[(k + 1) % plane_a_proj.size()];

					for (s32 l = 0; l < plane_b_proj.size(); l++)
					{
						auto& bp1 = plane_b_proj[l];
						auto& bp2 = plane_b_proj[(l + 1) % plane_b_proj.size()];

						auto rc = raycast(ap1, ap2 - ap1, bp1, bp2 - bp1);
						if (rc.result)
						{
							area.push_back(rc.contact);
						}
					}
				}

				for (s32 k = 0; k < plane_a_proj.size(); k++)
				{
					auto ap = plane_a_proj[k];
					if (is_inside(plane_b_proj[0], plane_b_proj[1], plane_b_proj[2], ap) ||
						is_inside(plane_b_proj[2], plane_b_proj[3], plane_b_proj[0], ap))
					{
						area.push_back(ap);
					}
				}

				for (s32 k = 0; k < plane_b_proj.size(); k++)
				{
					auto bp = plane_b_proj[k];
					if (is_inside(plane_a_proj[0], plane_a_proj[1], plane_a_proj[2], bp) ||
						is_inside(plane_a_proj[2], plane_a_proj[3], plane_a_proj[0], bp))
					{
						area.push_back(bp);
					}
				}


				if (area.empty())
				{
					continue;
				}

				area.points(make_convex_hull(area.points())); // TODO: fix this.

				const vec3& normal_a = plane_a.normal;
				const vec3& normal_b = plane_b.normal;

				vec3 normal_i = normalize(cross(normal_a, normal_b));

				// TODO: raycast and be parallel with line, and fail.
				vec3 point_i = raycast_to_plane(plane_a.vertices[0], plane_a.vertices[1], plane_b.vertices[0], normal_b);

				vec3 forward = { 0.0f, 0.0f, 1.0f };

				vec3 point_s = raycast_to_plane(plane_a.vertices[0] + forward, plane_a.vertices[1] + forward, plane_b.vertices[0], normal_b);

				vec2 proj_s = Renderer3D::project_point_to_view_plane(vec4(point_s, 1));
				vec2 proj_i = Renderer3D::project_point_to_view_plane(vec4(point_i, 1));
				vec2 proj_n = normalize(vec2(Renderer3D::project_point_to_view_plane(vec4(normal_i, 1))));

				vec2 proj = project_point_to_plane(proj_s, proj_i, proj_n).point;

				real slope = perp_dot(proj_n, proj - proj_s);

				result.intersections.push_back(
				{
					.plane_a = i,
					.plane_b = j,
					.area = area,
					.origin = point_i,
					.normal = normal_i,
					.slope = slope
				});
			}
		}

		return result;
	}

	real FormIntersectionsParallelMode::time_of_intersection(const vec2& point_on_overlapping_forms, const FormIntersections::Intersection& intersection)
	{
		vec2 p1 = Renderer3D::project_point_to_view_plane(vec4(intersection.origin, 1));
		vec2 p2 = Renderer3D::project_point_to_view_plane(vec4(intersection.origin + intersection.normal, 1));
		//vec2 n = normalize(Renderer3D::project_point_to_view_plane(vec4(intersection.normal, 1)));
		vec2 n = normalize(p2 - p1);

		vec2 proj = project_point_to_plane(point_on_overlapping_forms, p1, n).point;

		real slope = perp_dot(n, proj - point_on_overlapping_forms);
		slope /= intersection.slope;

		return slope;
	}

	pair<vec2, vec2> FormIntersectionsParallelMode::intersection_of_time(real toi, const FormIntersections::Intersection& intersection)
	{
		auto& area = intersection.area;
		vec2 p1 = Renderer3D::project_point_to_view_plane(vec4(intersection.origin, 1));
		vec2 p2 = Renderer3D::project_point_to_view_plane(vec4(intersection.origin + intersection.normal, 1));
		vec2 n = normalize(p2 - p1);

		vec2 sp = p1 + (vec2(-n.y, n.x) * intersection.slope) * toi;
		real min_t = INFINITY;
		real max_t = -INFINITY;

		for (s32 i = 0; i < area.size(); i++)
		{
			auto& l1 = area[i];
			auto& l2 = area[(i + 1) % area.size()];

			auto proj = project(sp, n, l1, (l2 - l1));
			if (proj.t2 >= 0.0f && proj.t2 < 1.0f)
			{
				min_t = MIN(proj.t1, min_t);
				max_t = MAX(proj.t1, max_t);
			}
		}

		if (min_t != INFINITY && max_t != -INFINITY && min_t != max_t)
		{
			return { { sp + n * min_t }, { sp + n * max_t } };
		}

		return { vec2(0), vec2(0) };
	}

	void FormIntersectionsParallelMode::clear_forms()
	{
		for (s32 i = 0; i < _forms.size(); i++)
		{
			auto& form = _forms[i];

			if (form != nullptr)
			{
				remove(form->name());
			}
		}

		_forms.clear();
		_debug_intersections.clear();
		_intersections.clear();
		_instersectionsDebugView->fill(0x00);
		_intersectionsOfTimeUDF->fill(0xFF); // TODO: may not need to fill iotudf on clear.
	}
}

