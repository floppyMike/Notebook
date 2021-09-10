#pragma once

#include <CustomLibrary/Collider.h>

#include "event.h"
#include "layout.h"
#include "window.h"

using namespace ctl;

inline auto start_selecting(const Renderer &r, WorldTextureDB &wts, mth::Point<float> wp) -> WorldTexture *
{
	const auto f =
		std::find_if(wts.begin(), wts.end(), [wp](const WorldTexture &wt) { return mth::collision(wt.dim, wp); });

	return f != wts.end() ? &*f : nullptr;
}

// ------------------------------------------------

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
		if (e.button.button == SDL_BUTTON_LEFT)
		{
			const auto wp = c.cam.screen_world(w.get_mousepos());

			WorldTexture *selected;
			CanvasType	  ct;

			if ((ct = STROKE, selected = start_selecting(r, c.swts, wp)) == nullptr)
				ct = TEXT, selected = start_selecting(r, c.txwts, wp);

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

inline void draw_selection(const Renderer &r, CanvasContext &c)
{
	if (c.select.wts != nullptr)
	{
		r.set_draw_color(sdl::BLUE);
		r.draw_rect(c.cam.world_screen(c.select.wts->dim));
	}
}
