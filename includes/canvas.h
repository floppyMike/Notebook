#pragma once

#include <CustomLibrary/SDL/All.h>

#include "comp/renderer.h"

#include "save.h"
#include "stroke.h"
#include "canvas/layout.h"

using namespace ctl;

class Canvas
{
public:
	Canvas() = default;

	void draw(const Renderer &r)
	{
		m_con.strokes.draw(r);
	}

	void event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r)
	{
		switch (e.type)
		{
		case SDL_MOUSEMOTION:
			if (ke.test(KeyEventMap::MOUSE_MIDDLE))
			{
				m_con.cam.translate(-e.motion.xrel, -e.motion.yrel);
				r.refresh();
			}

			break;

		case SDL_MOUSEWHEEL:
			zoom(w, m_con, e.wheel.y, 10.F);
			r.refresh();

			break;

		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
			case SDLK_s: save(&m_con); break;
			case SDLK_l: load(&m_con); break;
			}

			break;
		}

		m_con.strokes.event(e, ke, w, r, m_con.cam);
	}

private:
	CanvasContext m_con;
};
