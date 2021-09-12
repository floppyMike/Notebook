#pragma once

#include <CustomLibrary/IO.h>

#include "window.h"
#include "renderer.h"
#include "layout.h"

constexpr auto bar_size(int icon_n) -> mth::Dim<int>
{
	return { icon_n * (ICON_SIZE + SEPERATION) + SEPERATION, 2 * SEPERATION + ICON_SIZE };
}

inline auto mid_bottom(const Window &w, mth::Dim<int> d) -> mth::Point<int>
{
	const auto win = w.get_windowsize();
	return { (win.w - d.w) / 2, win.h - d.h };
}

inline auto gen_bar(Renderer &r, const TextureData &icon_map, IconDB &icons) -> Bar
{
	const auto bar = bar_size(icons.size());
	auto	   t   = r.create_texture(bar.w, bar.h);

	r.set_render_target(t);

	for (int i = 0; i < icons.size(); ++i)
	{
		mth::Rect<int> s = { i * ICON_SIZE, 0, ICON_SIZE, ICON_SIZE };
		mth::Rect<int> d = { SEPERATION + i * (ICON_SIZE + SEPERATION), SEPERATION, ICON_SIZE, ICON_SIZE };

		r.draw_frame(icon_map, s, d);
		icons[i].dim = d;
	}

	r.render_target();

	return { .dim = { 0, 0, bar.w, bar.h }, .data = std::move(t) };
}
