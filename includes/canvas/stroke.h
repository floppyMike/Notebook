#pragma once

#include <algorithm>

#include <CustomLibrary/Collider.h>

#include "window.h"
#include "line.h"
#include "texture.h"
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
auto shrink_to_fit(const Renderer &r, const CanvasContext &c) -> Texture<int>
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

	min.x -= c.target_line.radius;
	min.y -= c.target_line.radius;
	max.x += c.target_line.radius;
	max.y += c.target_line.radius;

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

	c.textures.erase(c.textures.end() - 1);
	c.lines.erase(c.lines.end() - 1);
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
	r.set_draw_color(c.target_line.color);

	r.draw_conn_stroke(p, c.target_line.radius);
	r.render_target();
	r.draw_conn_stroke(p, c.target_line.radius);
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
	const auto mp = w.get_mousepos();
	const auto w_size = w.get_windowsize();

	c.target_texture  = { .dim = { 0, 0, w_size.w, w_size.h }, .data = r.create_texture(w_size.w, w_size.h) };
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

	r.set_draw_color(c.target_line.color);

	r.draw_inter_stroke(c.target_line.points.back(), mp, c.target_line.radius);
	r.render_target();
	r.draw_inter_stroke(c.target_line.points.back(), mp, c.target_line.radius);
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

	c.lines.push_back({ c.target_line.radius, c.target_line.color, std::move(points) });

	c.target_line.points.clear();
	c.target_texture.data.reset();
}

auto find_intersections(const CanvasContext &c, const mth::Point<float> p) -> std::vector<size_t>
{
	std::vector<size_t> res;

	for (int i = c.textures.size() - 1; i >= 0; --i)
		if (mth::collision(p, c.textures[i].dim))
			res.emplace_back(i);

	res.erase(
		std::remove_if(res.begin(), res.end(),
					   [&c, p](const auto i)
					   {
						   return std::none_of(
							   c.lines[i].points.begin(), c.lines[i].points.end(),
							   [&c, i, p](const mth::Point<float> &l)
							   {
								   const mth::Rect box = { l.x, l.y, c.lines[i].radius * 2, c.lines[i].radius * 2 };
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
		auto &		texture = c.textures[i];
		const auto &line	= c.lines[i];

		auto t = r.create_texture(texture.dim.w, texture.dim.h);

		r.set_target(t);
		r.set_draw_color(line.color);

		const auto offset = texture.dim.pos();
		auto prev_pos = line.points.front() - offset;

		r.draw_conn_stroke(prev_pos, line.radius);

		for (auto p = line.points.begin() + 1; p != line.points.end(); ++p)
		{
			const auto next_pos = *p - offset;

			r.draw_inter_stroke(prev_pos, next_pos, line.radius);
			r.draw_conn_stroke(next_pos, line.radius);

			prev_pos = next_pos;
		}

		r.render_target(t);
		texture.data = std::move(t);
	}
}
