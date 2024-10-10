#pragma once

#include "window/backends/sdl/WindowSDL.h"
#include "node/Node2D.h"
#include "geometry/Quad.h"
#include "Polygon/Polygon.h"
#include "Polygon/Polygon3D.h"

#define COMPUTE_SHADER_LINE_DEFAULT_THICKNESS 0.005f
#define COMPUTE_SHADER_LINE_DEFAULT_EXPONENT 2.0f

namespace night
{

struct ComputeShaderParams
{
	s32 width{ 1000 };
	s32 height{ 1000 };
	Quad quad{ Quad(QuadParams{}) };
};

struct LineFragmentData
{
	Color8* pixel;
	ivec2 coordinate;
	real t;
};

struct PolygonFragmentData
{
	Color8* pixel;
	ivec2 coordinate;
	real tx;
	real ty;
	s32 distance;
	s32 max_distance;
};

struct Polygon3DFragmentData
{
	Color8* pixel;
	ivec2 coordinate; // TODO: add 3d global coordinate, get rid of depth
	real tx;
	real ty;
	s32 distance;
	s32 max_distance;
	real depth;
};

struct DrawLineParams
{
	vec2 p1{ 0 };
	vec2 p2{ 0 };
	real thickness1{ COMPUTE_SHADER_LINE_DEFAULT_THICKNESS };
	real thickness2{ COMPUTE_SHADER_LINE_DEFAULT_THICKNESS };
	real exponent{ COMPUTE_SHADER_LINE_DEFAULT_EXPONENT };
	Color color1{ WHITE }; // TODO: default black on white background.
	Color color2{ WHITE };
};

// TODO: make it so that transformation is handled by the renderer.
struct ComputeShader : public Node2D // TODO: add multithreading
{
	ComputeShader(const ComputeShaderParams& params = {});
	~ComputeShader();

	void fill(const Color& color);
	void fill(const Color8& color);
	void fill(u8 byte);

	// TODO: add thickness to draw_line / draw_pixel
	//void draw_line(const vec2& p1, const vec2& p2, const Color& color);
	void draw_line(const DrawLineParams& params);
	void draw_pixel(const vec2& point, const Color& color);

	s32 width() const; // TODO: store width and height in struct
	s32 height() const;

	Color8* pixels() const;
	const Color8& pixel(ivec2 coordinate) const;

	u8 is_internal_coordinate_in_bounds(const ivec2& coordinate) const;

	vec2 global_to_local(const vec2& global_coordinate) const;
	ivec2 local_to_internal(const vec2& local_coordinate) const;
	vec2 internal_to_global(const ivec2& internal_coordinate) const;

	vector<vector<ivec2>> contours() const;

	// LineFragmentData::pixel can be nullptr if fragment is out of bounds
	void rasterize_line(const ivec2& p1, const ivec2& p2, function<void(const LineFragmentData&)> fn) const; // TODO: add limit to how long line can be
	void rasterize_line(const vec2& p1, const vec2& p2, function<void(const LineFragmentData&)> fn) const;

	// TODO: fix bug where polygon in half off screen.
	void rasterize_polygon(const Polygon& polygon, function<void(const PolygonFragmentData&)> fn) const;
	void rasterize_polygon(const Polygon3D& polygon, function<void(const Polygon3DFragmentData&)> fn) const; // TODO: combine polygon3d and polygon

	void update_texture() const;

protected:

	virtual void on_render() override;

private:

	string _window_id;

	ref<ITexture> _texture;
	ref<ISurface> _surface;
	Quad _quad;
	mutable u8 _isPendingTextureUpdate{ false };
};

}