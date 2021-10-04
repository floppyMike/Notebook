#pragma once

#include "event.h"
#include "layout.h"

/**
 * @brief Regenerate modified text
 *
 * @param r Renderer for creating new text texture
 * @param tf Font used for text
 * @param stxi Text for generating
 *
 * @return Generated texture
 */
inline auto gen_text(Renderer &r, const TextFont &tf, const WorldTextInfo &stxi, mth::Point<float> loc)
	-> WorldTexture
{
	auto	   t = r.create_text(tf.data, stxi.str.c_str());
	const auto d = (mth::Dim<float>)r.get_texture_size(t) / stxi.scale; // Target world pos

	return WorldTexture{ .dim = { loc.x, loc.y, d.w, d.h }, .data = std::move(t) };
}

/**
 * @brief Regenerate whole text texture db
 *
 * @param r Create new text texture
 * @param tf Font for text to take on
 * @param wtxs Texture db to fill out
 * @param wtxis Text information for text
 */
inline void regen_texts(Renderer &r, const TextFont &tf, WorldTextureDB &wtxs, const WorldTextInfoDB &wtxis)
{
	assert(wtxis.size() == wtxs.size());

	for (size_t i = 0; i < wtxs.size(); ++i)
	{
		wtxs[i] = gen_text(r, tf, wtxis[i], wtxs[i].dim.pos());
	}
}

/**
 * @brief Add a character to the text information
 *
 * @param ch Character to add
 * @param stxi Info to fill out
 */
inline void add_character(char ch, WorldTextInfo &stxi)
{
	stxi.str.push_back(ch);
}

/**
 * @brief Remove a character when not empty
 * @param stxi Text info to modify
 * @return If text has no text
 */
inline void remove_character(WorldTextInfo &stxi)
{
	if (stxi.str.empty())
		return;

	stxi.str.pop_back();
}

/**
 * @brief Initialize a text texture together with info
 *
 * @param wp Text location
 * @param scale Camera scale to use for rendering
 *
 * @return Text and Info pair
 */
inline auto start_new_text(Renderer &r, TextFont &f, mth::Point<float> wp, float scale) -> std::pair<WorldTextInfo, WorldTexture>
{
	WorldTextInfo wtxi = { .str = "", .scale = scale };
	WorldTexture  wtx  = gen_text(r, f, wtxi, wp);

	return { std::move(wtxi), std::move(wtx) };
}

inline void start_text_input()
{
	SDL_StartTextInput();
	// SDL_FlushEvent(SDL_TEXTINPUT); // Text appears on previous keypress
	ctl::print("Start text input\n");
}

inline void stop_text_input()
{
	SDL_StopTextInput();
	ctl::print("End text input\n");
}

/**
 * @brief Add target text to texts
 *
 * @param etxt Text texture to move
 * @param etxi Text info to move
 * @param wtxts Text texture db to push to
 * @param wtxis Text info db to push to
 */
inline void flush_text(WorldTexture &etxt, WorldTextInfo &etxi, WorldTextureDB &wtxts, WorldTextInfoDB &wtxis)
{
	if (etxt.data == nullptr)
		return;

	wtxts.push_back(std::move(etxt));
	wtxis.push_back(std::move(etxi));
}
