#pragma once

#include "status.h"
#include "window.h"
#include "renderer.h"

#include "menu/layout.h"
#include "menu/bar.h"

/**
 * @brief Load the iconmap from file
 */
inline void load_iconmap(Renderer &r, BarContext &c)
{
	auto bmp = r.load_bmp("res/Icons.bmp");

	if (!bmp)
		throw std::runtime_error("File res/Icons.bmp not found.");

	c.icon_map = std::move(*bmp);
}

class Menu
{
public:
	void init(const Window &w, Renderer &r)
	{
		load_iconmap(r, c);

		const auto p = mid_bottom(w, c.bar.dim.dim());
		c.bar.dim.pos(p);
	}

	void draw(const Window &w, Renderer &r)
	{
		const auto p = mid_bottom(w, c.bar.dim.dim());
		c.bar.dim.pos(p);

		r.draw_texture(c.bar.data, c.bar.dim);
	}

	auto event(const SDL_Event &e, const Window &w, Renderer &r) -> bool
	{
		switch (e.type)
		{
		case EVENT_DRAW:
		case EVENT_SELECT:
			ctl::print("Bar select: %d\n", e.type);
			c.bar = gen_bar(r, c.icon_map, c.icons, e.type);
			r.refresh();

			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT:
				if (const auto mp = sdl::mouse_position(); mth::collision(c.bar.dim, mp))
					if (const auto f = intersect_bar(c.bar.dim.pos(), c.icons, mp); f != c.icons.end())
					{
						sdl::push_event(e.button.windowID, std::distance(c.icons.cbegin(), f));
						return false;
					}

				break;
			}

			break;
		}

		return true;
	}

private:
	BarContext c;
};
