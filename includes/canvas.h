#pragma once

#include <CustomLibrary/SDL/Camera2D.h>

#include "save.h"
#include "stroke.h"

using namespace ctl;

struct CanvasContext
{
	sdl::Camera2D cam;

	Strokes strokes;
};

void zoom(CanvasContext *c, int factor, float scale)
{
	mth::Point<int> mouse_p;
	SDL_GetMouseState(&mouse_p.x, &mouse_p.y);
	c->cam.zoom(1.F + factor / scale, mouse_p);
}

void save(const CanvasContext *c)
{
	pugi::xml_document doc;
	auto			   node = doc.append_child("doc");

	c->strokes.save(node);

	doc.save_file("save.xml");
}

void load(CanvasContext *c)
{
	pugi::xml_document doc;
	doc.load_file("save.xml");

	auto node = doc.first_child();

	c->strokes.load(node);
}

class Canvas
{
public:
	explicit Canvas(sdl::Window *w, sdl::Renderer *r)
		: m_con({ .strokes = Strokes(w, r, &m_con.cam) })
	{
	}

	void draw() { m_con.strokes.draw(); }

	void event(const SDL_Event &e, const KeyEvent &ke)
	{
		switch (e.type)
		{
		case SDL_MOUSEMOTION:
			if (ke.test(KeyEventMap::MOUSE_MIDDLE))
				m_con.cam.translate(-e.motion.xrel, -e.motion.yrel);
			break;

		case SDL_MOUSEWHEEL: zoom(&m_con, e.wheel.y, 10.f); break;

		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
			case SDLK_s: save(&m_con); break;
			case SDLK_l: load(&m_con); break;
			}

			break;
		}

		m_con.strokes.event(e, ke);
	}

private:
	CanvasContext m_con;
};
