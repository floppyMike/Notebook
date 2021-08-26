#pragma once

#include <CustomLibrary/SDL/All.h>

#include "renderer.h"

#include "canvas/layout.h"
#include "canvas/stroke.h"
#include "canvas/save.h"

using namespace ctl;

void handle_paint(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_UP:
			c.target_line_info.radius = std::min(c.target_line_info.radius + 1, 9);
			std::clog << "Radius: " << +c.target_line_info.radius << std::endl;
			r.set_stroke_radius(c.target_line_info.radius);

			break;

		case SDLK_DOWN:
			c.target_line_info.radius = std::max(c.target_line_info.radius - 1, 1);
			std::clog << "Radius: " << +c.target_line_info.radius << std::endl;
			r.set_stroke_radius(c.target_line_info.radius);

			break;

		case SDLK_r: c.target_line_info.color = sdl::RED; break;
		case SDLK_b: c.target_line_info.color = sdl::BLACK; break;

		case SDLK_s: save(c); break;

		case SDLK_l:
			clear(c);
			load(c);
			redraw(r, c);
			r.refresh();
			break;
		}

		break;

	case SDLK_y:
		c.status = TYPING;
		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_MIDDLE))
		{
			c.cam.translate((float)-e.motion.xrel, (float)-e.motion.yrel);
			r.refresh();
		}

		else if (ke.test(KeyEventMap::MOUSE_LEFT))
			continue_stroke(w, r, c);

		else if (ke.test(KeyEventMap::MOUSE_RIGHT))
			for (size_t i : find_intersections(c, c.cam.screen_world(mth::Point{ e.motion.x, e.motion.y })))
			{
				erase(c, i);
				r.refresh();
			}

		break;

	case SDL_MOUSEWHEEL:
		c.cam.zoom(1.F + (float)e.wheel.y / 10.F, (mth::Point<float>)w.get_mousepos());
		r.refresh();

		break;

	case SDL_MOUSEBUTTONDOWN:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT: trace_stroke(w, r, c); break;
		}

		break;

	case SDL_MOUSEBUTTONUP:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT: finalize_stroke(w, r, c); break;
		}

		break;
	}
}

class Canvas
{
public:
	Canvas(Renderer &r)
	{
		r.set_stroke_radius(m_con.target_line_info.radius);
	}

	void draw(const Renderer &r)
	{
		for (const auto &t : m_con.textures)
		{
			const auto world = m_con.cam.world_screen(mth::Rect{ t.dim.x, t.dim.y, t.dim.w, t.dim.h });
			r.draw_texture(t.data, world);
		}

		if (m_con.target_texture.data)
			r.draw_texture(m_con.target_texture.data, m_con.target_texture.dim);
	}

	void event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r)
	{
		switch (m_con.status)
		{
		case PAINTING: handle_paint(e, ke, w, r, m_con); break;
		default: break;
		};
	}

private:
	CanvasContext m_con;
};
