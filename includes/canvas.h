#pragma once

#include <CustomLibrary/SDL/All.h>

#include "comp/renderer.h"

#include "save.h"
#include "stroke.h"
#include "pipe.h"

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
	Canvas() = default;

	void draw(Renderer *r) { m_con.strokes.draw(r); }

	void event(EventPipe ep)
	{
		switch (ep.e.type)
		{
		case SDL_MOUSEMOTION:
			if (ep.ke.test(KeyEventMap::MOUSE_MIDDLE))
				m_con.cam.translate(-ep.e.motion.xrel, -ep.e.motion.yrel);
			break;

		case SDL_MOUSEWHEEL: zoom(&m_con, ep.e.wheel.y, 10.f); break;

		case SDL_KEYDOWN:
			switch (ep.e.key.keysym.sym)
			{
			case SDLK_s: save(&m_con); break;
			case SDLK_l: load(&m_con); break;
			}

			break;
		}

		m_con.strokes.event(ep);
	}

private:
	CanvasContext m_con;
};
