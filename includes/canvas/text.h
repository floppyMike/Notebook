#pragma once

#include "event.h"
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

// ------------------------------------------------

inline void handle_typing(const SDL_Event &e, const KeyEvent &ke, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			c.status = CanvasStatus::PAINTING;
			SDL_StopTextInput();
			break;

		case SDLK_BACKSPACE:
			remove_character(c.txwtxis[c.txe]);
			regen_text(r, c.txf, c.txwts[c.txe], c.txwtxis[c.txe]);

			r.refresh();

			break;

		case SDLK_RETURN:
			add_character('\n', c.txwtxis[c.txe]);
			regen_text(r, c.txf, c.txwts[c.txe], c.txwtxis[c.txe]);

			r.refresh();

			break;
		}

		break;

	case SDL_TEXTINPUT:
		add_character(e.text.text[0], c.txwtxis[c.txe]);
		regen_text(r, c.txf, c.txwts[c.txe], c.txwtxis[c.txe]);

		r.refresh();

		break;
	}
}

inline void draw_texts(const Renderer &r, CanvasContext &c)
{
	for (const auto &t : c.txwts)
	{
		const auto world = c.cam.world_screen(t.dim);
		r.draw_texture(t.data, world);
	}
}
