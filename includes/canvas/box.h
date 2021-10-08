#pragma once

#include <CustomLibrary/Collider.h>

#include "text.h"
#include "event.h"
#include "layout.h"
#include "window.h"

using namespace ctl;

/**
 * @brief Find a texture colliding with a world point
 *
 * @param ts Texture to compare with
 * @param wp World point to use
 *
 * @return std::find_if
 */
inline auto find_texture(WorldTextureDB &ts, mth::Point<float> wp)
{
	return std::find_if(ts.rbegin(), ts.rend(), [wp](const WorldTexture &wt) { return mth::collision(wt.dim, wp); });
}

/**
 * @brief Select a texture in a defined order (TODO)
 *
 * @param wts Strokes to search in
 * @param wtxs Texts to search in
 * @param wp 
 *
 * @return 
 */
inline auto start_selecting(WorldTextureDB &wts, WorldTextureDB &wtxs, mth::Point<float> wp) -> Select
{
	auto t = find_texture(wts, wp);

	if (t != wts.rend())
		return Select{ .idx = (size_t)std::distance(wts.begin(), t.base()) - 1, .wt = &*t, .type = CanvasType::STROKE };

	t = find_texture(wtxs, wp);

	if (t != wtxs.rend())
		return Select{ .idx = (size_t)std::distance(wtxs.begin(), t.base()) - 1, .wt = &*t, .type = CanvasType::TEXT };

	return Select{ .idx = (size_t)-1, .wt = nullptr, .type = CanvasType::NONE };
}

/**
 * @brief 
 * @param c
 */
inline void start_select(CanvasContext &c)
{
	ctl::print("Selecting\n");
	c.status = CanvasStatus::SELECTING;
}

/**
 * @brief 
 * @param c
 */
inline void stop_select(CanvasContext &c)
{
	if (c.select.type == CanvasType::TEXT)
		stop_text_input();

	c.select = Select{ .idx = (size_t)-1, .wt = nullptr, .type = CanvasType::NONE };
}

