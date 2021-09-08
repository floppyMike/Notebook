#pragma once

#include "layout.h"

inline void non_empty_regen(Renderer &r, const TextFont &tf, WorldTexture &wt, WorldTextInfo &stxi)
{
	auto	   t = r.create_text(tf.data, stxi.str.c_str());
	const auto d = (mth::Dim<float>)r.get_texture_size(t) / stxi.scale;
	wt			 = { .dim = { wt.dim.x, wt.dim.y, d.w, d.h }, .data = std::move(t) };
}

inline void regen_text(Renderer &r, const TextFont &tf, WorldTexture &wt, WorldTextInfo &wtxi)
{
	if (wtxi.str.empty())
	{
		wt.data.reset();
		return;
	}

	non_empty_regen(r, tf, wt, wtxi);
}

inline void regen_texts(Renderer &r, const TextFont &tf, WorldTextureDB &wtxs, WorldTextInfoDB &wtxis)
{
	for (size_t i = 0; i < wtxs.size(); ++i)
	{
		regen_text(r, tf, wtxs[i], wtxis[i]);
	}
}

inline void clear_texts(WorldTextureDB &wts, WorldTextInfoDB &wtxis)
{
	wts.clear();
	wtxis.clear();
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
