#pragma once

#include <CustomLibrary/IO.h>
#include <CustomLibrary/SDL/All.h>

#include "renderer.h"

#include "canvas/layout.h"
#include "canvas/stroke.h"
#include "canvas/save.h"
#include "canvas/text.h"

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
		case SDLK_UP: change_radius(r, c.target_line_info, c.target_line_info.radius + 1); break;
		case SDLK_DOWN: change_radius(r, c.target_line_info, c.target_line_info.radius - 1); break;

		case SDLK_r: c.target_line_info.color = sdl::RED; break;
		case SDLK_b: c.target_line_info.color = sdl::BLACK; break;

		case SDLK_s: save(c); break;

		case SDLK_l:
			clear(c);
			load(c);
			redraw(r, c.swts, c.swls, c.swlis);
			r.refresh();
			break;

		case SDLK_y: c.status = TYPING; break;
		}

		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_MIDDLE))
		{
			c.cam.translate((float)-e.motion.xrel, (float)-e.motion.yrel);
			r.refresh();
		}

		else if (ke.test(KeyEventMap::MOUSE_LEFT))
		{
			continue_stroke(w, r, c.target_texture, c.target_line, c.target_line_info);
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
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			std::tie(c.target_texture, c.target_line) = start_stroke(w, r, c.target_line_info);
			r.refresh();

			break;
		}

		break;

	case SDL_MOUSEBUTTONUP:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			c.target_texture   = finalize_stroke(w, r, c.target_texture, c.target_line, c.target_line_info);
			auto [wt, wl, wli] = transform_target_line(c.cam, c.target_texture, c.target_line, c.target_line_info);

			c.swts.push_back(std::move(wt));
			c.swls.push_back(std::move(wl));
			c.swlis.push_back(wli);

			clear_target_line(c.target_texture, c.target_line);

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

class Canvas
{
public:
	Canvas(Renderer &r)
	{
		r.set_stroke_radius(m_con.target_line_info.radius);
		m_con.target_font.data = r.create_font("/usr/share/fonts/TTF/NotoSansMono-Regular-Nerd-Font-Complete.ttf", 20);
	}

	void draw(const Renderer &r)
	{
		for (const auto &t : m_con.swts)
		{
			const auto world = m_con.cam.world_screen(t.dim);
			r.draw_texture(t.data, world);
		}

		if (m_con.target_texture.data)
			r.draw_texture(m_con.target_texture.data, m_con.target_texture.dim);

		for (const auto &t : m_con.texts)
		{
			const auto world = m_con.cam.world_screen(t.dim);
			r.draw_texture(t.data, world);
		}

		if (m_con.target_text.data)
			r.draw_texture(m_con.target_text.data, m_con.target_text.dim);
	}

	void event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r)
	{
		switch (m_con.status)
		{
		case PAINTING: handle_paint(e, ke, w, r, m_con); break;
		case TYPING: handle_typing(e, ke, r, m_con); break;
		default: break;
		};
	}

private:
	CanvasContext m_con;
};
