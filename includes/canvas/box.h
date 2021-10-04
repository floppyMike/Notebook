#pragma once

#include <CustomLibrary/Collider.h>

#include "event.h"
#include "layout.h"
#include "window.h"

using namespace ctl;

inline auto find_texture(WorldTextureDB &ts, mth::Point<float> wp)
{
	return std::find_if(ts.rbegin(), ts.rend(), [wp](const WorldTexture &wt) { return mth::collision(wt.dim, wp); });
}

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
