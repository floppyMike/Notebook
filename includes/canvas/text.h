#pragma once

#include "layout.h"

inline auto create_text_box(Renderer &r, const TextFont &tf, mth::Point<int> p, WorldTextInfo &stxi) -> ScreenTexture
{
	auto t = r.create_text(tf.data, stxi.str.c_str());
	auto d = r.get_texture_size(t);

	ScreenTexture st = { .dim = { p.x, p.y, d.w, d.h }, .data = std::move(t) };

	return st;
}

inline void regen_text(Renderer &r, const TextFont &tf, WorldTexture &wt, WorldTextInfo &stxi)
{
	if (stxi.str.empty())
	{
		wt.data.reset();
		return;
	}

	auto	   t = r.create_text(tf.data, stxi.str.c_str());
	const auto d = (mth::Dim<float>)r.get_texture_size(t) / stxi.scale;
	wt			 = { .dim = { wt.dim.x, wt.dim.y, d.w, d.h }, .data = std::move(t) };
}

inline void add_character(char ch, WorldTextInfo &stxi)
{
	stxi.str.push_back(ch);
}

inline void remove_character(WorldTextInfo &stxi)
{
	if (stxi.str.empty())
		return;

	stxi.str.pop_back();
}
