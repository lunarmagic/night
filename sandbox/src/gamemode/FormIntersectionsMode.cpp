
#include "nightpch.h"
#include "FormIntersectionsMode.h"
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

namespace night
{
	FormIntersectionsMode::FormIntersectionsMode(const FormIntersectionsModeParams& params)
	{
		//Renderer3D::view_angle(4);
		//Renderer3D::mode(EPerspectiveMode::Semi_Curvilinear);
		_params = params;
		Quad quad(QuadParams{});

		_wireframe = create<ComputeShader>("Form Intersections Mode Wire Frame Shader", ComputeShaderParams{
			 .width = params.internal_resolution.x,
			 .height = params.internal_resolution.y,
			 .quad = quad
			}
		);

		//_wireframe->visibility(ENodeVisibility::Invisible_Tree);
		_wireframe->depth(0.0f);

		_computeShader = create<ComputeShader>("Form Intersections Mode Compute Shader", ComputeShaderParams{
			 .width = params.internal_resolution.x,
			 .height = params.internal_resolution.y,
			 .quad = quad
			}
		);

		_computeShader->visibility(ENodeVisibility::Invisible_Tree);
		_computeShader->depth(1.0f);

		spawn_forms();

		BIND_INPUT(EKey::DELETE, EInputType::PRESSED, [&]() { reset(); });
		BIND_INPUT(EKey::RETURN, EInputType::PRESSED, [&]() { submit(); });

		_canvas = create<Canvas>("Form Intersections Mode Canvas", CanvasParams
			{
				.pen_color = _canvasPenColor,
				.compute_shader_params = { .width = params.internal_resolution.x, .height = params.internal_resolution.y }
			}
		);
		_canvas->depth(2.0f);
	}

	u8 FormIntersectionsMode::cull_normal(const vec3& normal, const vec3& point_on_plane) // TODO: use winding order
	{
		return dot(normal, point_on_plane - _cameraPosition) >= 0.0f;
	}

	void FormIntersectionsMode::on_render()
	{
		//_depthBuffer.render();
		//_wireframe.render();
		//_computeShader.render();

		//for (s32 i = 0; i < _intersections.size(); i++)
		//{
		//	auto& intersect = _intersections[i];
		//	for (s32 j = 0; j < intersect.lines.size(); j++)
		//	{
		//		Intersection::Line intersection = intersect.lines[j];

		//		//if (dot(__camera_direction, intersection.normal1) < 0.0f || dot(__camera_direction, intersection.normal2) < 0.0f)
		//		//{
		//		//	continue;
		//		//}
		//
		//		vec2 p1;
		//		vec2 p2;
		//
		//		p1 = Renderer3D::project_point_to_view_plane(vec4(intersection.p1.x, intersection.p1.y, intersection.p1.z, 1.0f));
		//		p2 = Renderer3D::project_point_to_view_plane(vec4(intersection.p2.x, intersection.p2.y, intersection.p2.z, 1.0f));
		//
		//		Window::get().draw_line(p1, p2, RED);
		//	}
		//}

		//_depthBuffer.clear();
		//
		//vec4 p1 = Renderer3D::project_point_to_view_plane(vec4(1.0, 0, 1.0, 1.0f));
		//vec4 p2 = Renderer3D::project_point_to_view_plane(vec4(-1.5, 0, 3, 1.0f));
		//
		//_depthBuffer.rasterize_line((vec2)p1, (vec2)p2, [&](const auto& fragment)
		//{
		//	if (fragment.pixel != nullptr)
		//	{
		//		Color c = RED;
		//		c.a *= 1.0f - lerp(p1.z, p2.z, fragment.t);
		//		c.a = CLAMP(c.a, 0, 1);
		//		Color8 color = Color8::make(c);
		//		*fragment.pixel = color;
		//	}
		//});
	}

	void FormIntersectionsMode::submit()
	{
		// calculate how close you were to the intersections.
		if (_computeShader == nullptr)
		{
			ERROR("FormIntersectionsMode _computeShader is nullptr!, name: ", name());
			return;
		}

		render_intersections();
		_computeShader->visibility(ENodeVisibility::Visible);
		//clear_forms(); // TODO: hide forms
	}

	void FormIntersectionsMode::reset()
	{
		clear_forms();
		spawn_forms();

		if (_computeShader == nullptr)
		{
			ERROR(name(), "'s _computeShader is nullptr!");
			return;
		}

		_computeShader->visibility(ENodeVisibility::Invisible_Tree);

		if (_canvas == nullptr)
		{
			ERROR(name(), "'s _canvas is nullptr!");
			return;
		}

		_canvas->clear();
	}

	void FormIntersectionsMode::spawn_forms()
	{
		if (_computeShader == nullptr/* || _wireframe == nullptr*/)
		{
			ERROR(name(), "'s _computeShader is nullptr!");
			return;
		}

		// Spawn in the forms with variability and density in there positions.
		for (s32 y = 0; y < _params.density.y; y++)
		{
			for (s32 x = 0; x < _params.density.x; x++)
			{
				real tx = (real)x / (real)(_params.density.x - 1);
				real ty = (real)y / (real)(_params.density.y - 1);

				auto& form = _forms.emplace_back(create<Box>("Form #" + to_string(x + y * _params.density.x), BoxParams{
					.transform = mat4(1),
					.extents = {1, 1, 1},
					.color = _wireframeColor
					}
				));

				if (form != nullptr)
				{
					form->visibility(ENodeVisibility::Invisible_Tree); // TODO: render forms to wireframe shader with z = line weight / edge depth.

					vec3 position = vec3(
						-_params.area.x / 2.0f + _params.area.x * tx + (random(_params.variation.x) - _params.variation.x / 2),
						-_params.area.y / 2.0f + _params.area.y * ty + (random(_params.variation.y) - _params.variation.y / 2),
						random(_params.variation.z) - _params.variation.z / 2
					);

					form->translate(position);
					form->rotate({ 1.0f, 0.0f, 0.0f }, random(R_PI)); // TODO: make random vector function.
					form->rotate({ 0.0f, 1.0f, 0.0f }, random(R_PI));
					form->rotate({ 0.0f, 0.0f, 1.0f }, random(R_PI));
				}
			}
		}

		// Find points of intersection between forms.
		for (s32 i = 0; i < _forms.size() - 1; i++)
		{
			for (s32 j = i + 1; j < _forms.size(); j++)
			{
				auto& a = _forms[i];
				auto& b = _forms[j];

				auto intersection = Intersection::intersect(a, b);
				_intersections.push_back(intersection);
			}
		}

		render_wireframe();

		//for (s32 i = 0; i < _intersections.size(); i++)
		//{
		//	auto& intersection = _intersections[i];
		//	for (s32 j = 0; j < intersection.lines.size(); j++)
		//	{
		//		auto& line = intersection.lines[j];
		//
		//		vec4 p1 = Renderer3D::project_point_to_view_plane(vec4(line.p1.x, line.p1.y, line.p1.z, 1.0f));
		//		vec4 p2 = Renderer3D::project_point_to_view_plane(vec4(line.p2.x, line.p2.y, line.p2.z, 1.0f));
		//		
		//		_depthBuffer.rasterize_line((vec2)p1, (vec2)p2, [&](const auto& fragment)
		//		{
		//			if (fragment.pixel != nullptr)
		//			{
		//				Color c = RED;
		//				c.a *= lerp(p1.z, p2.z, fragment.t) * 10.0f;
		//				Color8 color = Color8::make(c);
		//				*fragment.pixel = color;
		//			}
		//		});
		//	}
		//}
	}

	void FormIntersectionsMode::render_wireframe()
	{
		if (_wireframe == nullptr)
		{
			ERROR(name(), "'s _wireframe is nullptr!");
			return;
		}

		_wireframe->fill(0x00);

		vector<pair<vec4, vec4>> lines;
		lines.reserve(_forms.size() * 4 * 6 * 2);
		real min_depth = INFINITY;
		real max_depth = -INFINITY;

		for (s32 i = 0; i < _forms.size(); i++)
		{
			auto& form = _forms[i];
			if (form != nullptr)
			{
				auto planes = form->planes(); // TODO: optimize this.
				for (const auto& j : planes)
				{
					auto& vertices = j.vertices;
					for (s32 k = 0; k < vertices.size(); k++)
					{
						auto p1 = Renderer3D::project_point_to_view_plane(vec4(vertices[k], 1.0f));
						auto p2 = Renderer3D::project_point_to_view_plane(vec4(vertices[(k + 1) % vertices.size()], 1.0f));

						min_depth = MIN(p1.z, min_depth);
						max_depth = MAX(p1.z, max_depth);
						
						lines.emplace_back(p1, p2);


						//_wireframe->draw_line(
						//	{
						//		.p1 = p1,
						//		.p2 = p2,
						//		.thickness1 = CLAMP(NORMALIZE(p1.z, 1.0, 1.5), 0.0, 1.0) * 0.01, // TODO: calc min and max of the scene
						//		.thickness2 = CLAMP(NORMALIZE(p2.z, 1.0, 1.5), 0.0, 1.0) * 0.01,
						//		.color1 = _wireframeColor,
						//		.color2 = _wireframeColor
						//	}
						//);

						//_wireframe->rasterize_line((vec2)p1, (vec2)p2, [&](const auto& fragment)
						//{
						//	if (fragment.pixel == nullptr)
						//	{
						//		return;
						//	}
						//
						//	Color8 color = Color8(_wireframeColor);
						//	*fragment.pixel = color;
						//});
					}
				}
			}
		}

		for (s32 i = 0; i < lines.size(); i++)
		{
			auto& [p1, p2] = lines[i];

			real thickness1 = NORMALIZE(p1.z, min_depth, max_depth);
			thickness1 = CLAMP(thickness1, 0.0, 1.0);
			thickness1 = lerp(thickness1  * _wireframeThickness, _wireframeThickness, _wireframeThicknessDepth);

			real thickness2 = NORMALIZE(p2.z, min_depth, max_depth);
			thickness2 = CLAMP(thickness1, 0.0, 1.0);
			thickness2 = lerp(thickness1 * _wireframeThickness, _wireframeThickness, _wireframeThicknessDepth);
 
			_wireframe->draw_line(
				{
					.p1 = p1,
					.p2 = p2,
					.thickness1 = thickness1,
					.thickness2 = thickness2,
					.color1 = _wireframeColor,
					.color2 = _wireframeColor
				}
			);
		}
	}

	void FormIntersectionsMode::render_intersections()
	{
		// Generate depth buffer so we don't render intersections / outlines that are behind other forms
		_computeShader->fill(0x00);

		struct fragment_depth
		{
			real depth;
			ref<IForm> form;
		};

		vector<fragment_depth> depth_buffer(_computeShader->width() * _computeShader->height(), { .depth = -INFINITY });
		s32 dbw = _computeShader->width();

		real near_plane = 0.0f;
		real far_plane = 2.0f;

		// Create depth buffer
		for (s32 i = 0; i < _forms.size(); i++)
		{
			real t = (real)i / (real)(_forms.size()); // get color
			//t = CLAMP(t, 0.0, 0.99);

			auto& form = _forms[i];
			if (form != nullptr)
			{
				auto planes = form->planes();
				for (s32 j = 0; j < planes.size(); j++)
				{
					auto& plane = planes[j];

					if (cull_normal(plane.normal, plane.vertices[0])) // CULL BACKFACES
					{
						continue;
					}

					Polygon3D poly;

					for (s32 k = 0; k < plane.vertices.size(); k++)
					{
						auto& vertex = plane.vertices[k];
						poly.push_back(vec4(vertex.x, vertex.y, vertex.z, 1.0f));
					}

					_computeShader->rasterize_polygon(poly, [&](auto& fragment)
						{
							if (fragment.pixel != nullptr)
							{
								auto& depth = fragment.depth;
								auto& buffer = depth_buffer[fragment.coordinate.x + fragment.coordinate.y * dbw];

								if (depth > buffer.depth)
								{
									buffer.depth = depth;
									buffer.form = form;

									//Color8 color = Color8((Color::rainbow(t) * depth));
									//color.a /= 8;
									//*fragment.pixel = color;
								}
							}
						});
				}
			}
		}

		// Render intersections infront of there respective planes, but behind nearer forms
		for (s32 i = 0; i < _intersections.size(); i++)
		{
			auto& intersection = _intersections[i];
			for (s32 j = 0; j < intersection.lines.size(); j++)
			{
				auto& line = intersection.lines[j];

				//if (dot(__camera_direction, line.normal1) < 0.0f || dot(__camera_direction, line.normal2) < 0.0f)
				if (cull_normal(line.normal1, line.p1) || cull_normal(line.normal2, line.p1))
				{
					continue;
				}

				vec4 p1 = Renderer3D::project_point_to_view_plane(vec4(line.p1.x, line.p1.y, line.p1.z, 1.0f));
				vec4 p2 = Renderer3D::project_point_to_view_plane(vec4(line.p2.x, line.p2.y, line.p2.z, 1.0f));

				_computeShader->rasterize_line((vec2)p1, (vec2)p2, [&](const auto& fragment)
					{
						if (fragment.pixel == nullptr)
						{
							return;
						}

						constexpr real epsilon = 0.01f;
						real intersection_depth = lerp(p1.z, p2.z, fragment.t)/* + epsilon*/;
						auto& buffer = depth_buffer[fragment.coordinate.x + fragment.coordinate.y * dbw];

						if (intersection_depth < buffer.depth && buffer.form != intersection.a && buffer.form != intersection.b)
						{
							return;
						}

						Color8 color = Color8(_intersectionColor);
						*fragment.pixel = color;
					});
			}
		}

		// Render outlines of the forms.
		for (s32 i = 0; i < _forms.size(); i++)
		{
			auto& form = _forms[i];
			if (form != nullptr)
			{
				// contour compute shader.
				auto contour = form->contour();

				for (auto& i : contour)
				{
					i = Renderer3D::project_point_to_view_plane(i);
				}

				for (s32 j = 0; j < contour.size(); j++)
				{
					auto& p1 = contour[j];
					auto& p2 = contour[(j + 1) % contour.size()];

					_computeShader->rasterize_line((vec2)p1, (vec2)p2, [&](const auto& fragment)
					{
						if (fragment.pixel == nullptr)
						{
							return;
						}

						real intersection_depth = lerp(p1.z, p2.z, fragment.t);
						auto& buffer = depth_buffer[fragment.coordinate.x + fragment.coordinate.y * dbw];

						if (intersection_depth < buffer.depth && buffer.form != form)
						{
							return;
						}

						Color8 color = Color8(_contoursColor);
						*fragment.pixel = color;
					});
				}
			}
		}

		//_computeShader->draw_line(
		//	{
		//		.p1 = {-0.5f, 0.0f},
		//		.p2 = {0.5f, 0.2f},
		//		.thickness1 = 0.005f,
		//		.thickness2 = 0.025f,
		//		.color1 = RED,
		//		.color2 = GREEN
		//	}
		//);
	}

	void FormIntersectionsMode::clear_forms()
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
		_intersections.clear();
	}
}

