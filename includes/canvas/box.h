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
