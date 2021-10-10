#pragma once

#include "event.h"
#include "status.h"
#include "window.h"
#include "renderer.h"

#include "layout.h"
#include "stroke.h"
#include "save.h"
#include "text.h"
#include "box.h"

/**
 * @brief Check if the stroke has started
 */
inline auto stroke_started(const CanvasContext &c) -> bool
{
	return c.sst.data != nullptr;
}

/**
 * @brief Check if the erase line has started
 */
inline auto erase_started(const CanvasContext &c) -> bool
{
	return c.start_mp.has_value();
}

/**
 * @brief Add the drawn stroke to the db
 */
inline void add_stroke(CanvasContext &c)
{
	auto [wt, wl, wli] = transform_target_line(c.cam, c.sst, c.ssl, c.ssli);

	wl.idx = c.swls.size();
	c.wts.push_back(std::move(wt));

	c.swls.push_back(std::move(wl));
	c.swlis.push_back(wli);

	clear_target_line(c.sst, c.ssl);
}

/**
 * @brief Mark the start point of the erase line
 */
inline void start_erasing(CanvasContext &c)
{
	c.start_mp = c.cam.screen_world(sdl::mouse_position());
}

/**
 * @brief Erase all stroke on the erase line
 */
inline void erase_path(CanvasContext &c)
{
	const auto wp = c.cam.screen_world(sdl::mouse_position());

	auto col = find_line_intersections(c.wts, c.swls, mth::Line<float>::from(*c.start_mp, wp));

	for (size_t i : col) c.wts[c.swls[i].idx].data = nullptr;
	c.wts.erase(std::remove_if(c.wts.begin(), c.wts.end(), [](const WorldTexture &w) { return w.data == nullptr; }),
				c.wts.end());

	for (size_t i : col) erase(i, c.swls, c.swlis);
}

/**
 * @brief Initialize the painting subsystem
 */
inline void init_painting(CanvasContext &c)
{
	change_radius(c.cam, c.ssli, 3);
}

/**
 * @brief Handle all windows events for painting
 */
inline void handle_paint(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case EVENT_SELECT: start_select(c); break;

	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: change_radius(c.cam, c.ssli, c.ssli.i_rad + 1); break;
		case SDLK_DOWN: change_radius(c.cam, c.ssli, c.ssli.i_rad - 1); break;

		// Color selection shortcuts
		case SDLK_b: c.ssli.color = sdl::BLACK; break;
		case SDLK_r: c.ssli.color = sdl::RED; break;
		case SDLK_g: c.ssli.color = sdl::GREEN; break;
		case SDLK_o: c.ssli.color = sdl::ORANGE; break;
		}

		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_LEFT) && stroke_started(c))
		{
			continue_stroke(w, r, c.sst, c.ssl, c.ssli);
			r.refresh();
		}

		else if (ke.test(KeyEventMap::MOUSE_RIGHT) && erase_started(c))
			r.refresh();

		break;

	case SDL_MOUSEBUTTONDOWN:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			std::tie(c.sst, c.ssl) = start_stroke(w, r, c.ssli);
			r.refresh();

			break;

		case SDL_BUTTON_RIGHT:
			//
			start_erasing(c);

			break;
		}

		break;

	case SDL_MOUSEBUTTONUP:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			if (stroke_started(c))
			{
				c.sst = finalize_stroke(w, r, c.sst, c.ssl, c.ssli);
				add_stroke(c);
				r.refresh();
			}

			break;

		case SDL_BUTTON_RIGHT:
			if (erase_started(c))
			{
				erase_path(c);

				r.refresh();
				c.start_mp.reset();
			}

			break;
		}

		break;
	}
}

/**
 * @brief Draw the strokes to the window
 */
inline void draw_strokes(const Renderer &r, CanvasContext &c)
{
	for (const auto &t : c.wts)
	{
		const auto world = c.cam.world_screen(t.dim);
		r.draw_texture(t.data, world);
	}

	if (stroke_started(c))
		r.draw_texture(c.sst.data, c.sst.dim);

	if (c.status == CanvasStatus::PAINTING && erase_started(c))
	{
		const auto e = c.cam.world_screen(*c.start_mp);

		r.set_draw_color(sdl::GRAY);
		r.draw_line(e, sdl::mouse_position());
	}
}
