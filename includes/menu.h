#pragma once

#include "window.h"
#include "renderer.h"
#include "menu/layout.h"
#include "menu/bar.h"

class Menu
{
public:
	Menu(const Window &w, Renderer &r)
	{
		c.icon_map = r.load_bmp("res/Icons.bmp");
		c.icons	   = {
			   { .id = ID_DRAW },
			   { .id = ID_SELECT },
			   { .id = ID_TEXT },
		};

		c.bar = gen_bar(r, c.icon_map, c.icons);

		const auto p = mid_bottom(w, c.bar.dim.dim());
		c.bar.dim.pos(p);
	}

	void draw(const Window &w, Renderer &r)
	{
		const auto p = mid_bottom(w, c.bar.dim.dim());
		c.bar.dim.pos(p);

		r.draw_texture(c.bar.data, c.bar.dim);
	}

	void event(const SDL_Event &e)
	{
	}

private:
	BarContext c;
};
