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

// -----------------------------------------------------------------------------
// Text
// -----------------------------------------------------------------------------

/**
 * @brief Empty out the select data
 */
inline void reset_select(CanvasContext &c)
{
	c.select = Select{ .idx = (size_t)-1, .wt = nullptr, .type = CanvasType::NONE };
}

/**
 * @brief Recreate the text texture with new text
 */
inline void rebuild_text(Renderer &r, CanvasContext &c)
{
	c.txwts[c.select.idx] = gen_text(r, c.txf, c.txwtxis[c.select.idx], c.txwts[c.select.idx].dim.pos());
}

/**
 * @brief Load the default font for typing
 */
inline void init_typing(Renderer &r, CanvasContext &c)
{
	const char *font_path = "res/arial.ttf";
	auto		f		  = r.create_font(font_path, 30);

	if (!f)
		throw std::runtime_error("Font file not found.");

	c.txf.data = std::move(*f);
}

/**
 * @brief Handle windows events for typing
 */
inline void handle_typing(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_DELETE:
			erase(c.select.idx, c.txwts, c.txwtxis);

			stop_text_input();
			reset_select(c);

			break;

		case SDLK_BACKSPACE:
			remove_character(c.txwtxis[c.select.idx]);
			rebuild_text(r, c);

			r.refresh();

			break;

		case SDLK_RETURN:
			add_character('\n', c.txwtxis[c.select.idx]);
			rebuild_text(r, c);

			r.refresh();

			break;
		}

		break;

	case SDL_TEXTINPUT:
		add_character(e.text.text[0], c.txwtxis[c.select.idx]);
		rebuild_text(r, c);

		r.refresh();

		break;
	}
}

/**
 * @brief Draw the texts onto the window
 */
inline void draw_texts(const Renderer &r, CanvasContext &c)
{
	for (const auto &t : c.txwts)
	{
		const auto world = c.cam.world_screen(t.dim);
		r.draw_texture(t.data, world);
	}
}

// -----------------------------------------------------------------------------
// Selection
// -----------------------------------------------------------------------------

/**
 * @brief Check if a selection is occuring
 */
inline auto is_selected(CanvasContext &c) -> bool
{
	return c.select.wt != nullptr;
}

/**
 * @brief Move the selected texture by a delta
 */
inline void move_selected(CanvasContext &c, float dx, float dy)
{
	c.select.wt->dim.x += dx / c.cam.scale;
	c.select.wt->dim.y += dy / c.cam.scale;
}

/**
 * @brief Create a new empty text
 */
inline void push_empty_text(Renderer &r, CanvasContext &c, mth::Point<float> wp)
{
	auto [txi, txt] = start_new_text(r, c.txf, wp, c.cam.scale);

	c.txwts.push_back(std::move(txt));
	c.txwtxis.push_back(std::move(txi));

	start_text_input();
}

/**
 * @brief Initialize the selection subsystem
 */
inline void init_select()
{
}

/**
 * @brief Handle windows events for selecting
 */
inline void handle_selecting(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case EVENT_DRAW:
		stop_select(c);
		start_painting(c);

		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_LEFT))
		{
			if (is_selected(c))
			{
				move_selected(c, e.motion.xrel, e.motion.yrel);
				r.refresh();
			}
		}

		break;

	case SDL_MOUSEBUTTONUP:
		if (e.button.button == SDL_BUTTON_LEFT)
		{
			ctl::print("Reset temp line\n");
			c.start_mp.reset();
		}

		break;

	case SDL_MOUSEBUTTONDOWN:
		if (e.button.button == SDL_BUTTON_LEFT)
		{
			const auto wp = c.cam.screen_world(sdl::mouse_position());

			if (e.button.clicks == 2)
				push_empty_text(r, c, wp);

			c.select = start_selecting(c.swts, c.txwts, wp);

			ctl::print("Index: %d\n", c.select.idx);

			if (c.select.type == CanvasType::NONE)
			{
				ctl::print("Nothing selected\n");
				c.start_mp = wp;
			}
			else
			{
				ctl::print("Something selected\n");

				if (c.select.type != CanvasType::TEXT)
					stop_text_input();

				r.refresh();
			}
		}

		break;

	default:
		if (c.select.type == CanvasType::TEXT)
			handle_typing(e, ke, w, r, c);
	}
}

/**
 * @brief Draw the selection to the window
 */
inline void draw_selection(const Renderer &r, CanvasContext &c)
{
	if (is_selected(c))
	{
		r.set_draw_color(sdl::BLUE);
		r.draw_rect(c.cam.world_screen(c.select.wt->dim));
	}
}
