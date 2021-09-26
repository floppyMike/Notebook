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

	min.x -= sli.radius;
	min.y -= sli.radius;
	max.x += sli.radius;
	max.y += sli.radius;

	const auto w = max.x - min.x;
	const auto h = max.y - min.y;

	return { min.x, min.y, w, h };
}

/**
 * @brief Render joint between strokes
 *
 * @param r Get draw functions
 * @param st Get texture to draw to
 * @param sli Get radius information
 * @param p Point to draw the joint to
 */
inline void render_joint(Renderer &r, ScreenTexture &st, const ScreenLineInfo &sli, mth::Point<int> p)
{
	r.set_render_target(st.data);

	r.set_draw_color(sli.color);
	r.draw_joint_stroke(p, sli.radius);

	r.render_target();
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
inline void render_conn(Renderer &r, ScreenTexture &st, const ScreenLineInfo &sli, mth::Point<int> from,
						mth::Point<int> to)
{
	r.set_render_target(st.data);

	r.set_draw_color(sli.color);
	r.draw_con_stroke(from, to, sli.radius);

	r.render_target();
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

	ScreenTexture st = { .dim = { 0, 0, w_size.w, w_size.h }, .data = r.create_texture(w_size.w, w_size.h) };
	ScreenLine	  sl = { .points = { { mp.x, mp.y } } };

	render_joint(r, st, sli, mp);

	return { std::move(st), std::move(sl) };
}

/**
 * @brief Add & draw new location to target
 *
 * @param w Get mouse position
 * @param r Render line to texture and window
 * @param c Add target data
 */
inline void continue_stroke(const Window &w, Renderer &r, ScreenTexture &st, ScreenLine &sl, const ScreenLineInfo &sli)
{
	const auto mp = sdl::mouse_position();
	render_conn(r, st, sli, sl.points.back(), mp);
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
	render_joint(r, st, sli, sl.points.back());

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

	WorldLineInfo wli = { .radius = sli.radius / cam.scale, .scale = cam.scale, .color = sli.color };

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
inline auto find_line_intersections(const WorldTextureDB &wts, const WorldLineDB &wls, const WorldLineInfoDB &wlis,
									mth::Point<float> p) -> std::vector<size_t>
{
	std::vector<size_t> idx;

	for (size_t i = 0; i < wts.size(); ++i)
		if (mth::collision(p, wts[i].dim))
			idx.emplace_back(i);

	idx.erase(std::remove_if(idx.begin(), idx.end(),
							 [&wts, &wls, &wlis, p](const auto i)
							 {
								 return std::none_of(wls[i].points.begin(), wls[i].points.end(),
													 [offset = wts[i].dim.pos(), &wlis, i, p](mth::Point<float> l)
													 {
														 const mth::Rect<float> box = { l.x + offset.x, l.y + offset.y,
																						wlis[i].radius * 4,
																						wlis[i].radius * 4 };
														 return mth::collision(p, box);
													 });
							 }),
			  idx.end());

	return idx;
}

/**
 * @brief Generate textures using the stored list of lines
 *
 * @param r Draw & render lines onto textures
 * @param c Store the generated textures
 */
inline void redraw(Renderer &r, WorldTextureDB &wts, const WorldLineDB &wls, const WorldLineInfoDB &wlis)
{
	for (size_t i = 0; i < wts.size(); ++i)
	{
		auto &		wt	= wts[i];
		const auto &wl	= wls[i];
		const auto &wli = wlis[i];

		sdl::Camera2D cam{ .loc = { 0.F, 0.F }, .scale = wli.scale };

		const auto t_size = cam.world_screen(mth::Dim<float>{ wt.dim.w, wt.dim.h });
		auto	   t	  = r.create_texture(t_size.w, t_size.h);

		const auto rad = std::lrint(wli.radius * wli.scale);

		if (rad < 1)
			throw std::runtime_error("Stroke radius is under 1");

		r.set_draw_color(wli.color);
		r.set_stroke_radius(rad);
		r.set_render_target(t);

		auto prev_pos = cam.world_screen(wl.points.front());

		r.draw_joint_stroke(prev_pos, rad);

		for (auto p = wl.points.begin() + 1; p != wl.points.end(); ++p)
		{
			const auto next_pos = cam.world_screen(*p);
			r.draw_con_stroke(prev_pos, next_pos, rad);
			prev_pos = next_pos;
		}

		if (wl.points.size() > 1)
			r.draw_joint_stroke(prev_pos, rad);

		wt.data = std::move(t);
		r.render_target();
	}
}

inline void change_radius(Renderer &r, ScreenLineInfo &sli, int rad)
{
	sli.radius = std::clamp(rad, 1, 10);
	ctl::print("Radius: %i\n", sli.radius);
	r.set_stroke_radius(sli.radius);
}
