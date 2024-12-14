
#include "nightpch.h"
#include "FormIntersectionsParallelMode.h"
#include "form/Box.h"
#include "math/math.h"
#include "input/InputKey.h"
#include "input/EInput.h"
#include "application/Application.h"
#include "Renderer3D/Renderer3D.h"
#include "form/Intersection.h"
#include "geometry/Quad.h"
#include "Drawing/Canvas.h"
#include "algorithm/algorithm.h"
#include "gjk/gjk.h"
#include "form/IForm.h"
#include "form/Sphere.h"

namespace night
{
	function<void()> _test_render;

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

	void FormIntersectionsParallelMode::submit()
	{
		_score = 1.0f;

		// UDF
		struct _pixel_distance
		{
			u16 distance;
			u8 buffer;
			u8 alpha;
		};

		vector<ivec2> neighbor_queue;
		neighbor_queue.reserve(10000);

		vector<ivec2> next;
		next.reserve(10000);

		struct drawn_pixel
		{
			ivec2 coordinate{ 0 };
			s32 form_intersection{ -1 };
		};

		vector<drawn_pixel> drawn_intersection_pixels; // add which form intersection the pixels belong to.
		drawn_intersection_pixels.reserve(10000);

		auto rasterize_line = [&](const LineFragmentData& fragment)
		{
			if (fragment.pixel != nullptr && fragment.pixel->a != 0)
			{
				*fragment.pixel = { 0, 0, 255, 0 };
				neighbor_queue.emplace_back(fragment.coordinate);

				// add pixel to it's corresponding intersection overlap area.
				vec2 global = _canvas->compute_shader()->internal_to_global(fragment.coordinate);

				// TODO: fix this:
				drawn_intersection_pixels.push_back({ .coordinate = fragment.coordinate, .form_intersection = 0 });

				//for (s32 i = 0; i < _intersections.size(); i++)
				//{
				//	auto& fi = _intersections[i];
				//	
				//	u8 is_overlapping = false;
				//	for (s32 j = 0; j < fi.intersections.size(); j++)
				//	{
				//		auto& intersection = fi.intersections[j];
				//		
				//		// TODO: make this more accurate, don't compare truncated coordinate to real polygon.
				//		is_overlapping = gjk::overlap(&global, 1, intersection.area.points().data(), intersection.area.size());

				//		if (is_overlapping)
				//		{
				//			drawn_intersection_pixels.push_back({ .coordinate = fragment.coordinate, .form_intersection = i });
				//			return;
				//		}
				//	}
				//}
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

		// UDF QUERY
		struct IOTData
		{
			struct {
				real distance_sum{ 0 };
				s32 count{ 0 };
				real intersection_length{ 0 };
			} data[FORM_INTERSECTION_IOT_ACC];
		};

		constexpr real iot_range = 2.0f;
		
		for (s32 i = 0; i < _intersections.size(); i++)
		{
			auto& fi = _intersections[i];
			//vector<IOTData> iotd_sum;

			//for (s32 j = 0; j < fi.intersections.size(); j++)
			//{
			//	auto& intersection = fi.intersections[j];
			//	auto& iotd = iotd_sum.emplace_back();
			//	
			//	for (s32 k = 0; k < FORM_INTERSECTION_IOT_ACC; k++)
			//	{
			//		real t = (real)k / (real)(FORM_INTERSECTION_IOT_ACC - 1);
			//		auto& data = iotd.data[k];
			//		auto [ip1, ip2] = intersection_of_time(lerp(-iot_range, iot_range, t), intersection);
			//
			//		if (ip1 == ip2)
			//		{
			//			continue;
			//		}
			//
			//		data.intersection_length += distance(ip1, ip2);
			//
			//		_drawingUDF->rasterize_line(ip1, ip2, [&](auto& fragment)
			//		{
			//			if (fragment.pixel == nullptr)
			//			{
			//				return;
			//			}
			//
			//			_pixel_distance* dist = (_pixel_distance*)fragment.pixel;
			//			data.distance_sum += dist->distance/* / abs(intersection.slope)*/;
			//			data.count++;
			//		}
			//		);
			//	}
			//}

			//auto& iotd = iotd_sum.emplace_back();

			IOTData iotd;

			for (s32 k = 0; k < FORM_INTERSECTION_IOT_ACC; k++)
			{
				real t = (real)k / (real)(FORM_INTERSECTION_IOT_ACC - 1);
				auto intersections = intersection_of_time(lerp(-iot_range, iot_range, t), fi);
				auto& data = iotd.data[k];
				
				for (s32 j = 0; j < intersections.size(); j++)
				{
					auto& [ip1, ip2] = intersections[j];

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
				auto& data = iotd.data[j];

				if (data.count == 0)
				{
					continue;
				}

				real dist_sum = data.distance_sum / data.count;/* / (1.0f / inverse_dist_sum)*/;

				// INVERSE UDF:
				_intersectionsOfTimeUDF->fill(0xFF);
				neighbor_queue.clear();
				next.clear();
				auto intersections = intersection_of_time(lerp(-iot_range, iot_range, t), fi);

				for (s32 k = 0; k < intersections.size(); k++)
				{
					auto [ip1, ip2] = intersections[k];

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
				for (s32 k = 0; k < drawn_intersection_pixels.size(); k++)
				{
					//if (drawn_intersection_pixels[k].form_intersection == i) // TODO: use buckets for fi overlaps.
					{
						auto& coordinate = drawn_intersection_pixels[k].coordinate;

						if (_intersectionsOfTimeUDF->is_internal_coordinate_in_bounds(coordinate))
						{
							_pixel_distance& distance = (_pixel_distance&)_intersectionsOfTimeUDF->pixel(coordinate);
							inverse_dist_sum += (real)distance.distance;
							inverse_dist_count++;
						}
					}
				}

				if (inverse_dist_count != 0)
				{
					inverse_dist_sum /= inverse_dist_count;
				}

				dist_sum /= (1.0f / inverse_dist_sum);

				// UPDATE APPROX_TOI
				if (dist_sum < min_dist)
				{
					min_dist = dist_sum;
					approx_toi = lerp(-iot_range, iot_range, t); // TODO: store toi in data struct
				}
			}

			//for (s32 j = 0; j < FORM_INTERSECTION_IOT_ACC; j++)
			//{
			//	real t = (real)j / (real)(FORM_INTERSECTION_IOT_ACC - 1);
			//	real iot_dist_sum = 0;
			//	s32 iot_dist_count = 0;
			//
			//	for (s32 k = 0; k < iotd_sum.size(); k++)
			//	{
			//		auto& iotd = iotd_sum[k];
			//		auto& data = iotd.data[j];
			//
			//		if (data.count != 0)
			//		{
			//			// div by toi_length means bigger intersections give more points.
			//			iot_dist_sum += (data.distance_sum / data.count)/* / toi_length*/;
			//			iot_dist_count++;
			//		}
			//	}
			//
			//	if (iot_dist_count == 0)
			//	{
			//		continue;
			//	}
			//
			//	iot_dist_sum /= iot_dist_count;
			//
			//	// INVERSE UDF:
			//	_intersectionsOfTimeUDF->fill(0xFF);
			//	neighbor_queue.clear();
			//	next.clear();
			//
			//	for (s32 k = 0; k < fi.intersections.size(); k++)
			//	{
			//		auto& intersection = fi.intersections[k];
			//		auto [ip1, ip2] = intersection_of_time(lerp(-iot_range, iot_range, t), intersection);
			//
			//		if (ip1 == ip2)
			//		{
			//			continue;
			//		}
			//
			//		_intersectionsOfTimeUDF->rasterize_line(ip1, ip2, rasterize_line);
			//	}
			//
			//	process_udf(_intersectionsOfTimeUDF);
			//
			//	// iterate over all drawn pixels and compare them to the udf.
			//	real inverse_dist_sum = 0;
			//	s32 inverse_dist_count = 0;
			//	for (s32 k = 0; k < drawn_intersection_pixels.size(); k++)
			//	{
			//		if (drawn_intersection_pixels[k].form_intersection == i) // TODO: use buckets for fi overlaps.
			//		{
			//			auto& coordinate = drawn_intersection_pixels[k].coordinate;
			//
			//			if (_intersectionsOfTimeUDF->is_internal_coordinate_in_bounds(coordinate))
			//			{
			//				_pixel_distance& distance = (_pixel_distance&)_intersectionsOfTimeUDF->pixel(coordinate);
			//				inverse_dist_sum += (real)distance.distance;
			//				inverse_dist_count++;
			//			}
			//		}
			//	}
			//
			//	if (inverse_dist_count > 0)
			//	{
			//		inverse_dist_sum /= inverse_dist_count;
			//	}
			//
			//	TRACE("inverse_dist_sum: ", inverse_dist_sum);
			//
			//	// calc approx_toi:
			//	//real dist_sum = lerp(iot_dist_sum, inverse_dist_sum, 0.5f);
			//	real dist_sum = iot_dist_sum / (1.0f / inverse_dist_sum);
			//
			//	if (dist_sum < min_dist)
			//	{
			//		min_dist = dist_sum;
			//		approx_toi = lerp(-iot_range, iot_range, t); // TODO: store toi in data struct
			//	}
			//}

			fi.approx_toi = approx_toi;
			fi.distance_to_pixels = min_dist;

			TRACE("toi: ", fi.approx_toi, ", dist: ", fi.distance_to_pixels, ", inv: ",
				(1.0 / fi.distance_to_pixels));
		}

		// TODO: add correct amount of points to score

		_is_submitted = true;
	}

	void FormIntersectionsParallelMode::reset()
	{
		clear_forms();

		if (_canvas == nullptr)
		{
			ERROR(name(), "'s _canvas is nullptr!");
			return;
		}

		if (_drawingUDF == nullptr)
		{
			ERROR(name(), "'s _canvas is nullptr!");
			return;
		}

		_canvas->clear();
		_drawingUDF->fill(0xFF);
		_is_submitted = false;
		
		spawn_forms();
	}

	void FormIntersectionsParallelMode::spawn_forms()
	{
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

		//auto& form2 = _forms.emplace_back(create<Box>("Form #2", BoxParams{
		//		.transform = mat4(1),
		//		.extents = {1, 1, 1},
		//		.color = _wireframeColor
		//	}
		//));
		//
		//if (form2 != nullptr)
		//{
		//	vec3 position = vec3(
		//		-_params.area.x / 2.0f + _params.area.x * 0.5f + (random(_params.variation.x) - _params.variation.x / 2),
		//		-_params.area.y / 2.0f + _params.area.y * 0.5f + (random(_params.variation.y) - _params.variation.y / 2),
		//		0.0f
		//	);
		//
		//	form2->translate(position);
		//	form2->rotate({ 1.0f, 0.0f, 0.0f }, random(R_PI)); // TODO: make random vector function.
		//	form2->rotate({ 0.0f, 1.0f, 0.0f }, random(R_PI));
		//	form2->rotate({ 0.0f, 0.0f, 1.0f }, random(R_PI));
		//	form2->depth(5.0f);
		//}

		auto& form2 = _forms.emplace_back(create<Sphere>("Form #2", SphereParams{
				.transform = mat4(1),
				.radius = 1,
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
			form2->depth(5.0f);
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

				//FormIntersections intersection; // TODO: implement
				//intersection.form_a = a;
				//intersection.form_b = b;
				//_intersections.push_back(intersection);
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

	void draw_circle(vec3 point, vec3 normal, real radius)
	{
		constexpr s32 segments = 32;

		vec3 right = { 1.0f, 0.0f, 0.0f };
		vec4 perp_normal = vec4(normalize(cross(normal, right)), 1);

		for (s32 i = 0; i < segments; i++)
		{
			real t1 = (real)i / (real)(segments - 1) * R_PI * 2;
			real t2 = (real)(i + 1) / (real)(segments - 1) * R_PI * 2;

			vec3 p1 = perp_normal;
			p1 = glm::rotate(mat4(1), t1, normal) * vec4(p1, 1);

			vec3 p2 = perp_normal;
			p2 = glm::rotate(mat4(1), t2, normal) * vec4(p2, 1);

			p1 *= radius;
			p2 *= radius;
			p1 += point;
			p2 += point;

			utility::renderer().draw_line(Renderer3D::project_point_to_view_plane(vec4(p1, 1)), Renderer3D::project_point_to_view_plane(vec4(p2, 1)), BLUE);
		}
	};

	FormIntersectionsParallelMode::FormIntersections FormIntersectionsParallelMode::intersect(ref<IForm> form_a, ref<IForm> form_b)
	{
		TRACE("TODO: remove this function, use intersection_of_time.");
		FormIntersections result; // TODO: don't push empty intersections into the vector.

		result.form_a = form_a;
		result.form_b = form_b;

		//if (form_b->type() == EFormType::Sphere && form_a->type() == EFormType::Box)
		{
			// TODO: cull when there is no overlap of contours, and clip the intersection ellipse to the contour.
			// TODO: figure out the slope.
			// TODO: cull intersections on the backside of the ellipse.
			TRACE("TEST SPEHER");
			auto planes_a = ((ref<Box>)(form_a))->planes();
			auto sphere = (ref<Sphere>)form_b;
			vec4 sphere_origin = sphere->transform() * vec4(0, 0, 0, 1);
			real sphere_radius = sphere->radius();

			ProjectPointToPlaneResult3D projs[6];

			for (s32 i = 0; i < planes_a.size(); i++)
			{
				auto& plane = planes_a[i];
				auto projected = project_point_to_plane(vec3(sphere_origin), vec3(plane.vertices[0]), vec3(plane.normal));
				projs[i] = projected;
			}

			_test_render = [=]()
			{
				for (s32 i = 0; i < planes_a.size(); i++)
				{
					if (Renderer3D::should_cull_face(vec4(planes_a[i].vertices[0], 1), vec4(planes_a[i].vertices[1], 1), vec4(planes_a[i].vertices[2], 1)))
					{
						continue;
					}

					utility::renderer().draw_point(Renderer3D::project_point_to_view_plane(vec4(projs[i].point, 1)), RED);

					//real dist = distance(projs[i].point, vec3(sphere_origin));
					const real& dist = projs[i].distance;
					if (dist < sphere_radius) // we don't do this for slope.
					{
						real radius = sqrt(sphere_radius * sphere_radius - dist * dist);
						draw_circle(projs[i].point, planes_a[i].normal, radius);
					}
				}
				
			};

			return result;
		}
		/*else */if (form_a->type() != EFormType::Box || form_b->type() != EFormType::Box)
		{
			return result; // TODO: handle all form combos.
		}

		auto planes_a = ((ref<Box>)(form_a))->planes(); // TODO: figure out spheres.
		auto planes_b = ((ref<Box>)(form_b))->planes();

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

	vector<pair<vec2, vec2>> FormIntersectionsParallelMode::intersection_of_time(real toi, const FormIntersections& intersection)
	{
		vector<pair<vec2, vec2>> result; // TODO: don't push empty intersections into the vector.
		constexpr vec3 forward = { 0.0f, 0.0f, 1.0f };

		if (intersection.form_b->type() == EFormType::Sphere && intersection.form_a->type() == EFormType::Box)
		{
			// TODO: cull when there is no overlap of contours, and clip the intersection ellipse to the contour.
			// TODO: figure out the slope.
			// TODO: cull intersections on the backside of the ellipse.
			auto sphere = (ref<Sphere>)intersection.form_b;
			array<Box::Plane, 6> planes = ((ref<Box>)(intersection.form_a))->planes();
			vec4 sphere_origin = sphere->transform() * vec4(0, 0, 0, 1);
			real sphere_radius = sphere->radius();

			// APPLY TOI:
			sphere_origin += vec4(forward * toi, 0);
			for (s32 i = 0; i < planes.size(); i++)
			{
				auto& plane = planes[i];
				for (s32 j = 0; j < plane.vertices.size(); j++)
				{
					auto& vertex = plane.vertices[j];
					vertex -= forward * toi;
				}
			}

			// PROJECT SPHERE ORIGIN ONTO EACH PLANE OF THE BOX:
			ProjectPointToPlaneResult3D projs[6];

			for (s32 i = 0; i < planes.size(); i++)
			{
				auto& plane = planes[i];
				auto projected = project_point_to_plane(vec3(sphere_origin), vec3(plane.vertices[0]), vec3(plane.normal));
				projs[i] = projected;
			}

			// GENERATE INTERSECTION LINES:
			for (s32 i = 0; i < planes.size(); i++)
			{
				// cull planes that are facing away from camera.
				if (Renderer3D::should_cull_face(vec4(planes[i].vertices[0], 1), vec4(planes[i].vertices[1], 1), vec4(planes[i].vertices[2], 1)))
				{
					continue;
				}

				const real& dist = projs[i].distance;
				if (dist < sphere_radius) // we don't do this for slope.
				{
					real radius = sqrt(sphere_radius * sphere_radius - dist * dist);
					auto& normal = planes[i].normal;
					auto& point = projs[i].point;
					vec3 right = { 1.0f, 0.0f, 0.0f };
					vec4 perp_normal = vec4(normalize(cross(normal, right)), 1);
					constexpr s32 segments = 16;

					for (s32 i = 0; i < segments; i++)
					{
						real t1 = (real)i / (real)(segments - 1) * R_PI * 2;
						real t2 = (real)(i + 1) / (real)(segments - 1) * R_PI * 2;

						vec3 p1 = perp_normal;
						p1 = glm::rotate(mat4(1), t1, normal) * vec4(p1, 1);

						vec3 p2 = perp_normal;
						p2 = glm::rotate(mat4(1), t2, normal) * vec4(p2, 1);

						p1 *= radius;
						p2 *= radius;
						p1 += point;
						p2 += point;

						result.push_back({ Renderer3D::project_point_to_view_plane(vec4(p1, 1)), Renderer3D::project_point_to_view_plane(vec4(p2, 1)) });
					}
				}
			}

			//_test_render = [=]()
			//{
			//	for (s32 i = 0; i < planes.size(); i++)
			//	{
			//		if (Renderer3D::should_cull_face(vec4(planes[i].vertices[0], 1), vec4(planes[i].vertices[1], 1), vec4(planes[i].vertices[2], 1)))
			//		{
			//			continue;
			//		}
			//
			//		utility::renderer().draw_point(Renderer3D::project_point_to_view_plane(vec4(projs[i].point, 1)), RED);
			//
			//		//real dist = distance(projs[i].point, vec3(sphere_origin));
			//		const real& dist = projs[i].distance;
			//		if (dist < sphere_radius) // we don't do this for slope.
			//		{
			//			real radius = sqrt(sphere_radius * sphere_radius - dist * dist);
			//			draw_circle(projs[i].point, planes[i].normal, radius);
			//		}
			//	}
			//
			//};

			return result;
		}
		else if (intersection.form_a->type() != EFormType::Box || intersection.form_b->type() != EFormType::Box)
		{
			return result; // TODO: handle all form combos.
		}

		auto planes_a = ((ref<Box>)(intersection.form_a))->planes(); // TODO: figure out spheres.
		auto planes_b = ((ref<Box>)(intersection.form_b))->planes();

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

				vec3 point_s = raycast_to_plane(plane_a.vertices[0] + forward, plane_a.vertices[1] + forward, plane_b.vertices[0], normal_b);

				vec2 proj_s = Renderer3D::project_point_to_view_plane(vec4(point_s, 1));
				vec2 proj_i = Renderer3D::project_point_to_view_plane(vec4(point_i, 1));
				vec2 proj_n = normalize(vec2(Renderer3D::project_point_to_view_plane(vec4(normal_i, 1))));

				vec2 proj = project_point_to_plane(proj_s, proj_i, proj_n).point;

				real slope = perp_dot(proj_n, proj - proj_s);

				FormIntersections::Intersection intersection =
					{
						.plane_a = i,
						.plane_b = j,
						.area = area,
						.origin = point_i,
						.normal = normal_i,
						.slope = slope
					};

				result.push_back(intersection_of_time(toi, intersection));
			}
		}

		return result;
	}

	void FormIntersectionsParallelMode::on_render()
	{
		auto time_elapsed = utility::window().time_elapsed();
		//auto& current = _debug_intersections[s32(time_elapsed * 10) % _debug_intersections.size()];

		if (_is_submitted)
		{
			for (s32 i = 0; i < _intersections.size(); i++)
			{
				auto& fi = _intersections[i];

				constexpr real range = 3.5f;

				auto sweep = intersection_of_time((fmod(time_elapsed, range) - range / 2), fi);
				for (s32 j = 0; j < sweep.size(); j++)
				{
					auto& [p1, p2] = sweep[j];

					if (p1 != p2)
					{
						utility::renderer().draw_line(p1, p2, BLUE);
					}
				}

				auto approx = intersection_of_time(fi.approx_toi, fi);
				for (s32 j = 0; j < approx.size(); j++)
				{
					auto& [p1, p2] = approx[j];

					if (p1 != p2)
					{
						utility::renderer().draw_line(p1, p2, RED);
					}
				}

				//for (s32 j = 0; j < fi.intersections.size(); j++)
				//{
				//	auto& intersection = fi.intersections[j];
				//
				//	constexpr real range = 3.5f;
				//	auto [p1, p2] = intersection_of_time((fmod(time_elapsed, range) - range / 2), intersection);
				//
				//	if (p1 != p2)
				//	{
				//		utility::renderer().draw_line(p1, p2, BLUE);
				//	}
				//
				//	auto [ap1, ap2] = intersection_of_time(fi.approx_toi, intersection);
				//
				//	if (ap1 == ap2)
				//	{
				//		continue;
				//	}
				//
				//	utility::renderer().draw_line(ap1, ap2, RED);
				//}
			}
		}

		if (_test_render)
		{
			_test_render();
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
		//_debug_intersections.clear();
		_intersections.clear();
		_instersectionsDebugView->fill(0x00);
		_intersectionsOfTimeUDF->fill(0xFF); // TODO: may not need to fill iotudf on clear.
	}
}

