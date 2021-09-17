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
	ctl::print("Painting");
	c.status = CanvasStatus::PAINTING;
}

inline void init_typing(CanvasContext &c)
{
	ctl::print("Typing");
	c.status = CanvasStatus::TYPING;

	SDL_StartTextInput();
	SDL_FlushEvent(SDL_TEXTINPUT); // Text appears on previous keypress
}

inline void init_select(CanvasContext &c)
{
	ctl::print("Selecting");
	c.status = CanvasStatus::SELECTING;
}

inline void deinit_painting(CanvasContext &c)
{
}

inline void deinit_typing(CanvasContext &c)
{
	SDL_StopTextInput();
}

inline void deinit_select(CanvasContext &c)
{
	c.select.wts = nullptr;
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
		c.cam.zoom(1.F + (float)e.wheel.y / 10.F, w.get_mousepos());
		r.refresh();

		break;

	case EVENT_SAVE: save(c); break;

	case EVENT_LOAD:
		clear(c.swts, c.swls, c.swlis, c.txwts, c.txwtxis);

		load(c);

		redraw(r, c.swts, c.swls, c.swlis);
		regen_texts(r, c.txf, c.txwts, c.txwtxis);

		r.refresh();

		break;
	}
}

inline void handle_paint(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case EVENT_SELECT: init_select(c); break;
	case EVENT_TYPE: init_typing(c); break;

	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: change_radius(r, c.ssli, c.ssli.radius + 1); break;
		case SDLK_DOWN: change_radius(r, c.ssli, c.ssli.radius - 1); break;

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

		else if (ke.test(KeyEventMap::MOUSE_RIGHT))
			for (size_t i : find_line_intersections(c.swts, c.swls, c.swlis,
													c.cam.screen_world(mth::Point<int>{ e.motion.x, e.motion.y })))
			{
				erase(i, c.swts, c.swls, c.swlis);
				r.refresh();
			}

		break;

	case SDL_MOUSEWHEEL:
		c.cam.zoom(1.F + (float)e.wheel.y / 10.F, w.get_mousepos());
		r.refresh();

		break;

	case SDL_MOUSEBUTTONDOWN:
		if (e.button.button == SDL_BUTTON_LEFT)
		{
			std::tie(c.sst, c.ssl) = start_stroke(w, r, c.ssli);
			r.refresh();
		}

		break;

	case SDL_MOUSEBUTTONUP:
		if (e.button.button == SDL_BUTTON_LEFT && c.sst.data)
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
	}
}

inline void handle_typing(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_MOUSEBUTTONDOWN:
		if (e.button.button == SDL_BUTTON_LEFT)
		{
			const auto wp = c.cam.screen_world(w.get_mousepos());

			auto [wtxi, wtx] = start_typing(wp, c.cam.scale);
			c.txwtxis.push_back(std::move(wtxi));
			c.txwts.push_back(std::move(wtx));
			c.txe = c.txwts.size() - 1;

			r.refresh();
		}

		break;

	case EVENT_DRAW:
		deinit_typing(c);
		init_painting(c);

		break;

	case EVENT_SELECT:
		deinit_typing(c);
		init_select(c);

		break;

	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			deinit_typing(c);
			init_painting(c);

			break;

		case SDLK_BACKSPACE:
			remove_character(c.txwtxis[c.txe]);
			regen_text(r, c.txf, c.txwts[c.txe], c.txwtxis[c.txe]);

			r.refresh();

			break;

		case SDLK_RETURN:
			add_character('\n', c.txwtxis[c.txe]);
			regen_text(r, c.txf, c.txwts[c.txe], c.txwtxis[c.txe]);

			r.refresh();

			break;
		}

		break;

	case SDL_TEXTINPUT:
		add_character(e.text.text[0], c.txwtxis[c.txe]);
		regen_text(r, c.txf, c.txwts[c.txe], c.txwtxis[c.txe]);

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

	case EVENT_TYPE:
		deinit_select(c);
		init_typing(c);

		break;

	case SDL_MOUSEBUTTONDOWN:
		if (e.button.button == SDL_BUTTON_LEFT)
		{
			const auto wp = c.cam.screen_world(w.get_mousepos());

			WorldTexture *selected;
			CanvasType	  ct;

			if ((ct = CanvasType::STROKE, selected = start_selecting(r, c.swts, wp)) == nullptr)
				ct = CanvasType::TEXT, selected = start_selecting(r, c.txwts, wp);

			c.select = { .wts = selected, .type = ct };
			r.refresh();
		}

		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_LEFT) && c.select.wts != nullptr)
		{
			c.select.wts->dim.x += e.motion.xrel / c.cam.scale;
			c.select.wts->dim.y += e.motion.yrel / c.cam.scale;

			r.refresh();
		}

		break;
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
	if (c.select.wts != nullptr)
	{
		r.set_draw_color(sdl::BLUE);
		r.draw_rect(c.cam.world_screen(c.select.wts->dim));
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
		r.set_stroke_radius(c.ssli.radius);
#ifdef _WIN32
		m_con.txf.data = r.create_font("\\Windows\\Fonts\\arial.ttf", 30);
#else
		c.txf.data = r.create_font("/usr/share/fonts/TTF/DejaVuSans.ttf", 30);
#endif
	}

	void draw(const Renderer &r)
	{
		draw_strokes(r, c);
		draw_texts(r, c);
		draw_selection(r, c);

		debug_draw(r, c);
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
