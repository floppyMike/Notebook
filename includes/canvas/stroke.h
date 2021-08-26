#pragma once

#include <algorithm>

#include <CustomLibrary/Collider.h>

#include "window.h"
#include "event.h"
#include "layout.h"

using namespace ctl;

/**
 * @brief Shrink texture to draw area
 *
 * @param r Generate new texture
 * @param c	Access lines array
 *
 * @return Shrunk texture
 */
auto shrink_to_fit(const Renderer &r, const CanvasContext &c) -> ScreenTexture
{
	SDL_Point min = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
	SDL_Point max = { 0, 0 };

	for (const auto &p : c.target_line.points) // Find line range inside texture
	{
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
	}

	min.x -= c.target_line_info.radius;
	min.y -= c.target_line_info.radius;
	max.x += c.target_line_info.radius;
	max.y += c.target_line_info.radius;

	const auto w = max.x - min.x;
	const auto h = max.y - min.y;

	const mth::Rect rect = { static_cast<int>(min.x), static_cast<int>(min.y), static_cast<int>(w),
							 static_cast<int>(h) };

	return { .dim = { rect.x, rect.y, rect.w, rect.h }, .data = r.crop_texture(c.target_texture.data, rect) };
}

/**
 * @brief Quick erase texture at index
 *
 * @param c Using texture and lines array
 * @param i index to delete
 */
void erase(CanvasContext &c, size_t i)
{
	assert((c.textures.size() == c.lines.size()) && "Textures and lines not same size.");
	assert(!(c.textures.empty() || c.lines.empty()) && "Textures or lines empty.");

	std::swap(c.textures[i], c.textures.back());
	std::swap(c.lines[i], c.lines.back());
	std::swap(c.lines_info[i], c.lines_info.back());

	c.textures.erase(c.textures.end() - 1);
	c.lines.erase(c.lines.end() - 1);
	c.lines_info.erase(c.lines_info.end() - 1);
}

/**
 * @brief Render the start point
 *
 * @param r Draw onto texture
 * @param c Get texture and line data
 * @param p Get point to draw to
 */
void render_start_stroke(Renderer &r, CanvasContext &c, mth::Point<int> p)
{
	r.set_draw_color(c.target_line_info.color);

	r.draw_conn_stroke(p, c.target_line_info.radius);
	r.render_target();
	r.draw_conn_stroke(p, c.target_line_info.radius);
	r.render_target(c.target_texture.data);
}

/**
 * @brief Start drawing
 *
 * @param w Get relative mouse position
 * @param r Render to texture and window
 * @param c Fill out target data
 */
void trace_stroke(const Window &w, Renderer &r, CanvasContext &c)
{
	const auto mp	  = w.get_mousepos();
	const auto w_size = w.get_windowsize();

	c.target_texture = { .dim = { 0, 0, w_size.w, w_size.h }, .data = r.create_texture(w_size.w, w_size.h) };
	c.target_line.points.emplace_back(mp.x, mp.y);

	render_start_stroke(r, c, mp);
}

/**
 * @brief Add & draw new location to target
 *
 * @param w Get mouse position
 * @param r Render line to texture and window
 * @param c Add target data
 */
void continue_stroke(const Window &w, Renderer &r, CanvasContext &c)
{
	const auto mp = w.get_mousepos();

	r.set_draw_color(c.target_line_info.color);

	r.draw_inter_stroke(c.target_line.points.back(), mp, c.target_line_info.radius);
	r.render_target();
	r.draw_inter_stroke(c.target_line.points.back(), mp, c.target_line_info.radius);
	r.render_target(c.target_texture.data);

	c.target_line.points.emplace_back(mp);
}

/**
 * @brief Finalize target and store it
 *
 * @param r Texture manipulation
 * @param cam Calculating world
 * @param c Storing texture and line
 */
void finalize_stroke(const Window &w, Renderer &r, CanvasContext &c)
{
	render_start_stroke(r, c, w.get_mousepos());

	auto tex = shrink_to_fit(r, c);

	c.textures.push_back({ .dim = c.cam.screen_world(tex.dim), .data = std::move(tex.data) });

	std::vector<mth::Point<float>> points;
	points.reserve(c.target_line.points.size());
	for (auto p : c.target_line.points) points.emplace_back(c.cam.screen_world(p));

	c.lines.push_back({ std::move(points) });
	c.lines_info.push_back({ c.target_line_info.radius, c.cam.scale(), c.target_line_info.color });

	c.target_line.points.clear();
	c.target_texture.data.reset();
}

/**
 * @brief Find all points intersecting with given point
 *
 * @param c Get all lines
 * @param p Point to compare to
 *
 * @return Collection of indexes for collisions
 */
auto find_intersections(const CanvasContext &c, const mth::Point<float> p) -> std::vector<size_t>
{
	std::vector<size_t> res;

	for (int i = c.textures.size() - 1; i >= 0; --i)
		if (mth::collision(p, c.textures[i].dim))
			res.emplace_back(i);

	res.erase(std::remove_if(res.begin(), res.end(),
							 [&c, p](const auto i)
							 {
								 return std::none_of(c.lines[i].points.begin(), c.lines[i].points.end(),
													 [&c, i, p](const mth::Point<float> &l)
													 {
														 const mth::Rect box = { l.x, l.y,
																				 c.lines_info[i].radius * 2 + 1,
																				 c.lines_info[i].radius * 2 + 1 };
														 return mth::collision(p, box);
													 });
							 }),
			  res.end());

	return res;
}

/**
 * @brief Generate textures using the stored list of lines
 *
 * @param r Draw & render lines onto textures
 * @param c Store the generated textures
 */
void redraw(Renderer &r, CanvasContext &c)
{
	for (size_t i = 0; i < c.textures.size(); ++i)
	{
		auto &		tex = c.textures[i];
		const auto &l	= c.lines[i];
		const auto &lf	= c.lines_info[i];

		sdl::Camera2D cam(tex.dim.pos(), lf.scale);

		const auto t_size = cam.world_screen(mth::Dim{ tex.dim.w, tex.dim.h });
		auto	   t	  = r.create_texture(t_size.w, t_size.h);

		r.set_draw_color(lf.color);
		r.set_stroke_radius(lf.radius);

		auto prev_pos = cam.world_screen(l.points.front());

		r.draw_conn_stroke(prev_pos, lf.radius);

		for (auto p = l.points.begin() + 1; p != l.points.end(); ++p)
		{
			const auto next_pos = cam.world_screen(*p);
			r.draw_inter_stroke(prev_pos, next_pos, lf.radius);
			prev_pos = next_pos;
		}

		if (l.points.size() > 1)
			r.draw_conn_stroke(prev_pos, lf.radius);

		r.render_target(t);
		tex.data = std::move(t);
	}

	r.set_stroke_radius(c.target_line_info.radius);
}

/**
 * @brief Clear all data from canvas
 *
 * @param c Access the data to be cleared
 */
void clear(CanvasContext &c)
{
	c.textures.clear();
	c.lines.clear();
	c.lines_info.clear();
}
