#pragma once

#include <algorithm>

#include <CustomLibrary/IO.h>
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
inline auto get_line_dim(const Renderer &r, const ScreenLine &sl, const ScreenLineInfo &sli) -> mth::Rect<int>
{
	SDL_Point min = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
	SDL_Point max = { 0, 0 };

	for (const auto &p : sl.points) // Find line range inside texture
	{
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
	}

	const auto rad = sli.radius;

	min.x -= (int)rad;
	min.y -= (int)rad;
	max.x += (int)rad;
	max.y += (int)rad;

	const auto w = max.x - min.x;
	const auto h = max.y - min.y;

	return { min.x, min.y, w, h };
}

/**
 * @brief Render a stroke connection between 2 strokes
 *
 * @param r Get draw functions
 * @param st Get texture to draw to
 * @param sli Get radius information
 * @param from Point to draw from
 * @param to Point to draw to
 */
inline void render_conn(Renderer &r, ScreenTexture &st, const ScreenLineInfo &sli, mth::Line<int> path)
{
	const auto abs = path.abs_rect();

	const mth::Rect<int> area = { abs.x - (int)sli.radius, abs.y - (int)sli.radius, abs.w + (int)sli.radius * 2,
								  abs.h + (int)sli.radius * 2 };

	r.set_stroke_target(st.data, area, sli.radius);

	r.set_stroke_color(sli.color);
	r.draw_stroke(path.pos1(), path.pos2());

	r.render_stroke(st.data);
}

/**
 * @brief Start drawing
 *
 * @param w Get relative mouse position
 * @param r Render to texture and window
 * @param c Fill out target data
 */
inline auto start_stroke(const Window &w, Renderer &r, const ScreenLineInfo &sli)
	-> std::pair<ScreenTexture, ScreenLine>
{
	const auto mp	  = sdl::mouse_position();
	const auto w_size = w.get_windowsize();

	auto t = r.create_stroke_texture(w_size.w, w_size.h);
	r.render_stroke(t);

	ScreenTexture st = { .dim = { 0, 0, w_size.w, w_size.h }, .data = std::move(t) };
	ScreenLine	  sl = { .points = { { mp.x, mp.y } } };

	render_conn(r, st, sli, mth::Line<int>::from(mp, mp));

	return { std::move(st), std::move(sl) };
}

/**
 * @brief Add & draw new location to target if not a duplicate
 *
 * @param w Get mouse position
 * @param r Render line to texture and window
 * @param c Add target data
 */
inline void continue_stroke(const Window &w, Renderer &r, ScreenTexture &st, ScreenLine &sl, const ScreenLineInfo &sli)
{
	const auto mp = sdl::mouse_position();

	if (mp == sl.points.back()) // Some systems (like linux) have multiple events...
		return;

	render_conn(r, st, sli, mth::Line<int>::from(sl.points.back(), mp));
	sl.points.push_back(mp);
}

/**
 * @brief Finalize target and store it
 *
 * @param r Texture manipulation
 * @param cam Calculating world
 * @param c Storing texture and line
 *
 * @return Finished stroke
 */
inline auto finalize_stroke(const Window &w, Renderer &r, ScreenTexture &st, ScreenLine &sl, const ScreenLineInfo &sli)
	-> ScreenTexture
{
	const auto line_dim = get_line_dim(r, sl, sli);
	auto	   tex		= r.crop_texture(st.data, line_dim);

	return { .dim = line_dim, .data = std::move(tex) };
}

/**
 * @brief Transform screen line to a world line
 *
 * @param cam Get the relativistic position of the screen
 * @param st Screen size & texture
 * @param sl Screen lines of stroke
 * @param sli Screen radius, scale & color
 *
 * @return World line combo
 */
inline auto transform_target_line(const sdl::Camera2D &cam, ScreenTexture &st, ScreenLine &sl,
								  const ScreenLineInfo &sli) -> std::tuple<WorldTexture, WorldLine, WorldLineInfo>
{
	WorldTexture wt = { .dim = cam.screen_world(st.dim), .data = std::move(st.data) };

	WorldLine wl = { .points = std::vector<mth::Point<float>>(sl.points.size()) };
	std::transform(sl.points.begin(), sl.points.end(), wl.points.begin(),
				   [&cam, &wt](mth::Point<int> p) { return cam.screen_world(p) - wt.dim.pos(); });

	WorldLineInfo wli = { .radius = sli.radius, .scale = cam.scale, .color = sli.color };

	return { std::move(wt), std::move(wl), wli };
}

/**
 * @brief Clear info from screen line
 *
 * @param st Screen texture
 * @param sl Screen line
 */
inline void clear_target_line(ScreenTexture &st, ScreenLine &sl)
{
	st.data.reset();
	sl.points.clear();
}

/**
 * @brief Find all points intersecting with given point
 *
 * @param c Get all lines
 * @param p Point in world to compare to
 *
 * @return Collection of indexes for collisions
 */
inline auto find_line_intersections(const WorldTextureDB &wts, const WorldLineDB &wls, mth::Line<float> ml)
	-> std::vector<size_t>
{
	std::vector<size_t> idx;

	// for (const WorldLine &wl : wls)
	for (size_t i = 0; i < wls.size(); ++i)
	{
		const auto &ps	= wls[i].points;
		const auto &tex = wts[wls[i].idx];

		if (mth::collision(ml, tex.dim))
		{
			if (ps.size() < 5) // If dot-like texture (also protects search)
			{
				idx.push_back(i);
				continue;
			}

			for (auto ii = ps.begin(); ii != ps.end() - 1; ++ii)
				if (mth::collision(mth::Line<float>::from(*ii + tex.dim.pos(), *(ii + 1) + tex.dim.pos()), ml))
				{
					idx.push_back(i);
					break;
				}
		}
	}

	return idx;
}

/**
 * @brief Generate textures using the stored list of lines
 *
 * @param r Draw & render lines onto textures
 * @param c Store the generated textures
 */
inline void regen_strokes(Renderer &r, WorldTextureDB &wts, const WorldLineDB &wls, const WorldLineInfoDB &wlis)
{
	for (size_t i = 0; i < wts.size(); ++i)
	{
		auto		 &wt	= wts[i];
		const auto &wl	= wls[i];
		const auto &wli = wlis[i];

		sdl::Camera2D cam{ .loc = { 0.F, 0.F }, .scale = wli.scale };

		const auto t_size = cam.world_screen(mth::Dim<float>{ wt.dim.w, wt.dim.h });
		auto	   t	  = r.create_stroke_texture(t_size.w, t_size.h);
		const auto rad	  = wli.radius;

		r.set_stroke_color(wli.color);
		r.set_stroke_target(t, { 0, 0, t_size.w, t_size.h }, rad);

		std::vector<mth::Point<int>> ps_pos(wl.points.size());
		std::transform(wl.points.begin(), wl.points.end(), ps_pos.begin(),
					   [&cam](mth::Point<float> p) { return cam.world_screen(p); });

		r.draw_stroke_multi(ps_pos);

		r.render_stroke(t);
		wt.data = std::move(t);
	}
}

/**
 * @brief Change the on screen stroke radius
 *
 * @param cam Calculate stroke radius relative to camera
 * @param sli Load the new stroke radius into
 * @param rad New stroke radius
 */
inline void change_radius(const sdl::Camera2D &cam, ScreenLineInfo &sli, int rad)
{
	if (sli.i_rad != rad)
	{
		sli.i_rad = std::clamp(rad, 1, 10);
		ctl::print("Radius: %i\n", sli.i_rad);
	}

	sli.radius = sli.i_rad * cam.scale;
}

/**
 * @brief Called when painting is to be done
 */
inline void start_painting(CanvasContext &c)
{
	ctl::print("Painting\n");
	c.status = CanvasStatus::PAINTING;
}

/**
 * @brief Called when painting has stopped and the state is switched
 */
inline void stop_painting(CanvasContext &c)
{
}
