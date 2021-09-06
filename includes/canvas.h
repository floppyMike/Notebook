#pragma once

#include <CustomLibrary/IO.h>
#include <CustomLibrary/SDL/All.h>

#include "renderer.h"

#include "canvas/layout.h"
#include "canvas/stroke.h"
#include "canvas/save.h"
#include "canvas/text.h"
#include "canvas/box.h"

using namespace ctl;

inline void change_radius(Renderer &r, ScreenLineInfo &sli, int rad)
{
	sli.radius = std::clamp(rad, 1, 10);
	ctl::print("Radius: %i\n", sli.radius);
	r.set_stroke_radius(rad);
}

inline void handle_paint(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: change_radius(r, c.ssli, c.ssli.radius + 1); break;
		case SDLK_DOWN: change_radius(r, c.ssli, c.ssli.radius - 1); break;

		case SDLK_r: c.ssli.color = sdl::RED; break;
		case SDLK_b: c.ssli.color = sdl::BLACK; break;

		case SDLK_y: c.status = TYPING; break;
		case SDLK_v: c.status = SELECTING; break;
		}

		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_LEFT))
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

	case SDL_MOUSEBUTTONDOWN:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			std::tie(c.sst, c.ssl) = start_stroke(w, r, c.ssli);
			r.refresh();

			break;
		}

		break;

	case SDL_MOUSEBUTTONUP:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			c.sst			   = finalize_stroke(w, r, c.sst, c.ssl, c.ssli);
			auto [wt, wl, wli] = transform_target_line(c.cam, c.sst, c.ssl, c.ssli);

			c.swts.push_back(std::move(wt));
			c.swls.push_back(std::move(wl));
			c.swlis.push_back(wli);

			clear_target_line(c.sst, c.ssl);

			r.refresh();

			break;
		}

		break;
	}
}

inline void handle_typing(const SDL_Event &e, const KeyEvent &ke, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_y: c.status = PAINTING; break;
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
		create_text_box(r, c);
		r.refresh();
		break;
	}
}

inline void handle_selecting(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_v:
			c.status	 = PAINTING;
			c.select.wts = nullptr;
			break;
		}

		break;

	case SDL_MOUSEBUTTONDOWN:
		const auto mp = w.get_mousepos();
		c.select	  = { .wts = start_selecting(r, c.swts, c.cam.screen_world(mp)), .type = STROKE };
		r.refresh();

		break;
	}
}

class Canvas
{
public:
	Canvas(Renderer &r)
	{
		r.set_stroke_radius(m_con.ssli.radius);
		m_con.target_font.data = r.create_font("/usr/share/fonts/TTF/NotoSansMono-Regular-Nerd-Font-Complete.ttf", 20);
	}

	void draw(const Renderer &r)
	{
		for (const auto &t : m_con.swts)
		{
			const auto world = m_con.cam.world_screen(t.dim);
			r.draw_texture(t.data, world);
		}

		if (m_con.sst.data)
			r.draw_texture(m_con.sst.data, m_con.sst.dim);

		for (const auto &t : m_con.texts)
		{
			const auto world = m_con.cam.world_screen(t.dim);
			r.draw_texture(t.data, world);
		}

		if (m_con.target_text.data)
			r.draw_texture(m_con.target_text.data, m_con.target_text.dim);

		if (m_con.select.wts != nullptr)
		{
			r.set_draw_color(sdl::BLUE);
			r.draw_rect(m_con.cam.world_screen(m_con.select.wts->dim));
		}
	}

	void event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r)
	{
		switch (e.type)
		{
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
			case SDLK_s: save(m_con); break;

			case SDLK_l:
				clear(m_con);
				load(m_con);
				redraw(r, m_con.swts, m_con.swls, m_con.swlis);
				r.refresh();
				break;
			}

			break;

		case SDL_MOUSEMOTION:
			if (ke.test(KeyEventMap::MOUSE_MIDDLE))
			{
				m_con.cam.translate((float)-e.motion.xrel, (float)-e.motion.yrel);
				r.refresh();
			}
			break;

		case SDL_MOUSEWHEEL:
			m_con.cam.zoom(1.F + (float)e.wheel.y / 10.F, w.get_mousepos());
			r.refresh();

			break;
		}

		switch (m_con.status)
		{
		case PAINTING: handle_paint(e, ke, w, r, m_con); break;
		case TYPING: handle_typing(e, ke, r, m_con); break;
		case SELECTING: handle_selecting(e, ke, w, r, m_con); break;

		default: break;
		};
	}

private:
	CanvasContext m_con;
};
