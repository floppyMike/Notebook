#pragma once

#include <charconv>
#include "layout.h"

inline void debug_init(Renderer &r, CanvasContext &c)
{
	c.debug.mouse = r.create_text(c.txf.data, "0 0");
}

inline void debug_event(const SDL_Event &e, Renderer &r, CanvasContext &c)
{
#ifndef NDEBUG
	switch (e.type)
	{
	case SDL_MOUSEMOTION:
		char b[32];

		const auto wp = c.cam.screen_world(sdl::mouse_position());
		auto *	   p  = std::to_chars(b, b + 32, wp.x).ptr;
		*p			  = ' ';
		++p;
		*std::to_chars(p, b + 32, wp.y).ptr = '\0';

		c.debug.mouse = r.create_text(c.txf.data, b);

		r.refresh();
	}
#endif
}

inline void debug_draw(const Renderer &r, const CanvasContext &c)
{
#ifndef NDEBUG
	const auto s = r.get_texture_size(c.debug.mouse);
	r.draw_texture(c.debug.mouse, { 0, 0, s.w, s.h });
#endif
}
