#pragma once

#include "renderer.h"
#include "status.h"

#include "canvas/layout.h"
#include "canvas/stroke.h"
#include "canvas/save.h"
#include "canvas/text.h"
#include "canvas/box.h"
#include "canvas/debug.h"

using namespace ctl;

// -----------------------------------------------------------------------------
// Init
// -----------------------------------------------------------------------------

inline void init_painting(CanvasContext &c)
{
	ctl::print("Painting\n");
	c.status = CanvasStatus::PAINTING;
}

inline void init_typing(CanvasContext &c)
{
	ctl::print("Typing\n");
	c.status = CanvasStatus::TYPING;
}

inline void init_select(CanvasContext &c)
{
	ctl::print("Selecting\n");
	c.status = CanvasStatus::SELECTING;
}

inline void deinit_painting(CanvasContext &c)
{
}

inline void deinit_typing(CanvasContext &c)
{
	// flush_text(c.txet, c.txei, c.txwts, c.txwtxis);
}

inline void deinit_select(CanvasContext &c)
{
	if (c.select.type == CanvasType::TEXT)
		stop_text_input();

	c.select = Select{ .idx = (size_t)-1, .wt = nullptr, .type = CanvasType::NONE };
}

// -----------------------------------------------------------------------------
// Event Handlers
// -----------------------------------------------------------------------------

inline void handle_general(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_MIDDLE))
		{
			c.cam.translate((float)-e.motion.xrel, (float)-e.motion.yrel);
			r.refresh();
		}

		break;

	case SDL_MOUSEWHEEL:
	{
		const auto s = std::clamp(c.cam.scale * (1.F + (float)e.wheel.y / 10.F), 0.1F, 10.F);
		c.cam.set_zoom(s, sdl::mouse_position());
		change_radius(r, c.cam, c.ssli, c.ssli.i_rad);

		r.refresh();

		break;
	}

	case EVENT_SAVE:
		if (const auto filename = open_file_save(); filename)
			save(c, filename->c_str());

		break;

	case EVENT_LOAD:
		if (const auto filename = open_file_load(); filename)
		{
			clear(c.swts, c.swls, c.swlis, c.txwts, c.txwtxis);

			CATCH_LOG(load(c, filename->c_str()));

			redraw(r, c.swts, c.swls, c.swlis);
			regen_texts(r, c.txf, c.txwts, c.txwtxis);

			r.refresh();
		}

		break;
	}
}

inline void handle_paint(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case EVENT_SELECT: init_select(c); break;

	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: change_radius(r, c.cam, c.ssli, c.ssli.i_rad + 1); break;
		case SDLK_DOWN: change_radius(r, c.cam, c.ssli, c.ssli.i_rad - 1); break;

		case SDLK_r: c.ssli.color = sdl::RED; break;
		case SDLK_b: c.ssli.color = sdl::BLACK; break;
		}

		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_LEFT) && c.sst.data)
		{
			continue_stroke(w, r, c.sst, c.ssl, c.ssli);
			r.refresh();
		}

		else if (ke.test(KeyEventMap::MOUSE_RIGHT) && c.start_mp)
		{
			r.refresh();
		}

		break;

	case SDL_MOUSEBUTTONDOWN:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			std::tie(c.sst, c.ssl) = start_stroke(w, r, c.ssli);
			r.refresh();

			break;

		case SDL_BUTTON_RIGHT: c.start_mp = c.cam.screen_world(sdl::mouse_position()); break;
		}

		break;

	case SDL_MOUSEBUTTONUP:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			if (c.sst.data)
			{
				c.sst			   = finalize_stroke(w, r, c.sst, c.ssl, c.ssli);
				auto [wt, wl, wli] = transform_target_line(c.cam, c.sst, c.ssl, c.ssli);

				c.swts.push_back(std::move(wt));
				c.swls.push_back(std::move(wl));
				c.swlis.push_back(wli);

				clear_target_line(c.sst, c.ssl);

				r.refresh();
			}

			break;

		case SDL_BUTTON_RIGHT:
			if (c.start_mp)
			{
				const auto wp = c.cam.screen_world(sdl::mouse_position());

				auto col = find_line_intersections(c.swts, c.swls, c.swlis, mth::Line<float>::from(*c.start_mp, wp));
				std::sort(col.rbegin(), col.rend()); // Avoid deletion of empty cells

				for (size_t i : col) erase(i, c.swts, c.swls, c.swlis);

				r.refresh();
				c.start_mp.reset();
			}

			break;
		}

		break;
	}
}

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
			c.select = Select{ .idx = (size_t)-1, .wt = nullptr, .type = CanvasType::NONE };

			break;

		case SDLK_BACKSPACE:
			remove_character(c.txwtxis[c.select.idx]);
			c.txwts[c.select.idx] = gen_text(r, c.txf, c.txwtxis[c.select.idx], c.txwts[c.select.idx].dim.pos());

			r.refresh();

			break;

		case SDLK_RETURN:
			add_character('\n', c.txwtxis[c.select.idx]);
			c.txwts[c.select.idx] = gen_text(r, c.txf, c.txwtxis[c.select.idx], c.txwts[c.select.idx].dim.pos());

			r.refresh();

			break;
		}

		break;

	case SDL_TEXTINPUT:
		add_character(e.text.text[0], c.txwtxis[c.select.idx]);
		c.txwts[c.select.idx] = gen_text(r, c.txf, c.txwtxis[c.select.idx], c.txwts[c.select.idx].dim.pos());

		r.refresh();

		break;
	}
}

inline void handle_selecting(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case EVENT_DRAW:
		deinit_select(c);
		init_painting(c);

		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_LEFT))
		{
			if (c.select.wt != nullptr)
			{
				c.select.wt->dim.x += e.motion.xrel / c.cam.scale;
				c.select.wt->dim.y += e.motion.yrel / c.cam.scale;

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
			{
				auto [txi, txt] = start_new_text(r, c.txf, wp, c.cam.scale);

				c.txwts.push_back(std::move(txt));
				c.txwtxis.push_back(std::move(txi));

				start_text_input();
			}

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

// -----------------------------------------------------------------------------
// Draw
// -----------------------------------------------------------------------------

inline void draw_strokes(const Renderer &r, CanvasContext &c)
{
	for (const auto &t : c.swts)
	{
		const auto world = c.cam.world_screen(t.dim);
		r.draw_texture(t.data, world);
	}

	if (c.sst.data)
		r.draw_texture(c.sst.data, c.sst.dim);

	if (c.status == CanvasStatus::PAINTING && c.start_mp)
	{
		const auto e = c.cam.world_screen(*c.start_mp);

		r.set_draw_color(sdl::GRAY);
		r.draw_line(e, sdl::mouse_position());
	}
}

inline void draw_texts(const Renderer &r, CanvasContext &c)
{
	for (const auto &t : c.txwts)
	{
		const auto world = c.cam.world_screen(t.dim);
		r.draw_texture(t.data, world);
	}
}

inline void draw_selection(const Renderer &r, CanvasContext &c)
{
	if (c.select.wt != nullptr)
	{
		r.set_draw_color(sdl::BLUE);
		r.draw_rect(c.cam.world_screen(c.select.wt->dim));
	}
}

// -----------------------------------------------------------------------------
// Canvas
// -----------------------------------------------------------------------------

class Canvas
{
public:
	void init(Renderer &r)
	{
		change_radius(r, c.cam, c.ssli, 3);

		const char *font_path = "res/arial.ttf";
		auto		f		  = r.create_font(font_path, 30);

		if (!f)
			throw std::runtime_error("Font file not found.");

		c.txf.data = std::move(*f);

		debug_init(r, c);
	}

	void draw(const Renderer &r)
	{
		draw_strokes(r, c);
		draw_texts(r, c);
		draw_selection(r, c);

		debug_draw(r, c.cam, c);
	}

	bool event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r)
	{
		handle_general(e, ke, w, r, c);

		switch (c.status)
		{
		case CanvasStatus::PAINTING: handle_paint(e, ke, w, r, c); break;
		case CanvasStatus::SELECTING: handle_selecting(e, ke, w, r, c); break;
		case CanvasStatus::TYPING: handle_typing(e, ke, w, r, c); break;
		};

		debug_event(e, r, c);

		return true;
	}

private:
	CanvasContext c;
};
