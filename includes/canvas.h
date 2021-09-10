#pragma once

#include <CustomLibrary/SDL/All.h>

#include "renderer.h"

#include "canvas/layout.h"
#include "canvas/stroke.h"
#include "canvas/save.h"
#include "canvas/text.h"
#include "canvas/box.h"

using namespace ctl;

// -----------------------------------------------------------------------------
// Common
// -----------------------------------------------------------------------------

inline void handle_save(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	if (e.type == SDL_KEYDOWN)
		switch (e.key.keysym.sym)
		{
		case SDLK_s: save(c); break;

		case SDLK_l:
			clear(c.swts, c.swls, c.swlis, c.txwts, c.txwtxis);

			load(c);

			redraw(r, c.swts, c.swls, c.swlis);
			regen_texts(r, c.txf, c.txwts, c.txwtxis);

			r.refresh();

			break;
		}
}

inline void handle_camera(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
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
	}
}

// -----------------------------------------------------------------------------
// Canvas
// -----------------------------------------------------------------------------

class Canvas
{
public:
	Canvas(Renderer &r)
	{
		r.set_stroke_radius(m_con.ssli.radius);
#ifdef _WIN32
		m_con.txf.data = r.create_font("\\Windows\\Fonts\\arial.ttf", 30);
#else
		m_con.txf.data = r.create_font("/usr/share/fonts/TTF/DejaVuSans.ttf", 30);
#endif
	}

	void draw(const Renderer &r)
	{
		draw_strokes(r, m_con);
		draw_texts(r, m_con);
		draw_selection(r, m_con);
	}

	void event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r)
	{
		handle_camera(e, ke, w, r, m_con);

		switch (m_con.status)
		{
		case PAINTING: handle_paint(e, ke, w, r, m_con); break;
		case SELECTING: handle_selecting(e, ke, w, r, m_con); break;

		case TYPING: handle_typing(e, ke, r, m_con); return;
		};

		handle_save(e, ke, w, r, m_con);
	}

private:
	CanvasContext m_con;
};
