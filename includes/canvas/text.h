#pragma once

#include "layout.h"

inline void create_text_box(Renderer &r, CanvasContext &c)
{
	auto t = r.create_text(c.target_font.data, "<Type something in>");

	mth::Dim<int> d;
	SDL_QueryTexture(t.get(), nullptr, nullptr, &d.w, &d.h);
	auto p = sdl::mouse_position();

	c.target_text = { .dim = { p.x, p.y, d.w, d.h }, .data = std::move(t) };
}

