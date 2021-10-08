#pragma once

#include "renderer.h"
#include "status.h"

#include "canvas/debug.h"

#include "canvas/general_handler.h"
#include "canvas/stroke_handler.h"
#include "canvas/selection_handler.h"

using namespace ctl;

class Canvas
{
public:
	void init(Renderer &r)
	{
		init_painting(c);
		init_select();
		init_typing(r, c);

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
