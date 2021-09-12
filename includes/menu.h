#pragma once

#include "menu/layout.h"
#include "window.h"
#include "renderer.h"

constexpr auto bar_dim(int icon_n) -> mth::Dim<int>
{
	return { icon_n * (ICON_SIZE + SEPERATION) + SEPERATION, 2 * SEPERATION + ICON_SIZE };
}

inline auto rescale(const Window &w, size_t icon_n) -> mth::Point<int>
{
	const auto win = w.get_windowsize();
	const auto bar = bar_dim(icon_n);

	return { (win.w - bar.w) / 2, win.h - bar.h };
}

class Menu
{
public:
	Menu(const Window &w, Renderer &r)
	{
		c.icon_map = r.load_bmp("res/Icons.bmp");
		c.icons	   = { { .dim = { 0, 0, 32, 32 }, .id = ID_DRAW },
					   { .dim = { 32, 0, 32, 32 }, .id = ID_SELECT },
					   { .dim = { 64, 0, 32, 32 }, .id = ID_TEXT } };

		c.bar_loc = rescale(w, c.icons.size());
	}

	void draw(const Window &w, Renderer &r)
	{
		c.bar_loc = rescale(w, c.icons.size());

		for (int i = 0; i < c.icons.size(); ++i)
		{
			mth::Rect<int> s = { i * ICON_SIZE, 0, ICON_SIZE, ICON_SIZE };
			mth::Rect<int> d = { c.bar_loc.x + SEPERATION + i * (ICON_SIZE + SEPERATION), c.bar_loc.y + SEPERATION,
								 ICON_SIZE, ICON_SIZE };

			r.draw_frame(c.icon_map, s, d);
		}
	}

	void event(const SDL_Event &e)
	{
	}

private:
	BarContext c;
};
