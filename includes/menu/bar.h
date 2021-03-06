#pragma once

#include <CustomLibrary/IO.h>
#include <CustomLibrary/Collider.h>

#include "window.h"
#include "renderer.h"
#include "layout.h"

/**
 * @brief Calculate the necessary bar size to fill in for icons
 * @param icon_n icon amount
 * @return Dimensions of the box
 */
constexpr auto bar_size(int icon_n) -> mth::Dim<int>
{
	return { icon_n * (ICON_SIZE + SEPERATION) + SEPERATION, 2 * SEPERATION + ICON_SIZE };
}

/**
 * @brief Align the box to the bottom of the screen
 *
 * @param w Get the window size
 * @param d Bar size
 *
 * @return Screen location for bar
 */
inline auto mid_bottom(const Window &w, mth::Dim<int> d) -> mth::Point<int>
{
	const auto win = w.get_windowsize();
	return { (win.w - d.w) / 2, win.h - d.h };
}

/**
 * @brief Generate the bar texture
 *
 * @param r Renderer to draw to
 * @param icon_map Texture map of icons
 * @param icons Icons configuration
 * @param selection selected icon
 *
 * @return Bar texture
 */
inline auto gen_bar(Renderer &r, const Renderer::Texture &icon_map, IconDB &icons, size_t selection) -> Bar
{
	const auto bar = bar_size((int)icons.size());
	auto	   t   = r.create_texture(bar.w, bar.h);

	r.set_render_target(t);

	for (int i = 0; i < icons.size(); ++i)
	{
		mth::Rect<int> s = { i * ICON_SIZE, 0, ICON_SIZE, ICON_SIZE };
		mth::Rect<int> d = { SEPERATION + i * (ICON_SIZE + SEPERATION), SEPERATION, ICON_SIZE, ICON_SIZE };

		r.draw_frame(icon_map, s, d);
		icons[i].dim = d;
	}
	
	assert(icons.size() > selection);

	r.set_draw_color(sdl::GREEN);
	r.draw_rect(icons[selection].dim);

	r.render_target();

	return { .dim = { 0, 0, bar.w, bar.h }, .data = std::move(t) };
}

inline auto intersect_bar(mth::Point<int> rel, const IconDB &icons, mth::Point<int> p)
{
	return std::find_if(
		icons.begin(), icons.end(),
		[p, rel](const Icon &i) {
			return mth::collision(p, mth::Rect<int>{ i.dim.x + rel.x, i.dim.y + rel.y, i.dim.w, i.dim.h });
		});
}
