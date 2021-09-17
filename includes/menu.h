#pragma once

#include "status.h"
#include "window.h"
#include "renderer.h"

#include "menu/layout.h"
#include "menu/bar.h"

class Menu
{
public:
	void init(const Window &w, Renderer &r)
	{
		c.icon_map = r.load_bmp("res/Icons.bmp");
		c.icons	   = {
			   { .id = EVENT_DRAW }, { .id = EVENT_SELECT }, { .id = EVENT_TYPE },
			   { .id = EVENT_SAVE }, { .id = EVENT_LOAD },
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

	auto event(const SDL_Event &e, const Window &w, Renderer &r) -> bool
	{
		switch (e.type)
		{
		case SDL_MOUSEMOTION:
			if (mth::collision(c.bar.dim, mth::Point<int>{ e.motion.x, e.motion.y }))
				if (const auto f = intersect_bar(c.bar.dim.pos(), c.icons, w.get_mousepos()); f != c.icons.end())
					c.hover = &*f;
				else
					c.hover = nullptr;
			else
				c.hover = nullptr;

			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT:
				if (c.hover)
				{
					SDL_Event event = sdl::create_event(e.button.windowID, c.hover->id);
					SDL_PushEvent(&event);

					if (c.hover->id != EVENT_SAVE && c.hover->id != EVENT_LOAD)
					{
						c.bar = gen_bar(r, c.icon_map, c.icons, c.hover);
						r.refresh();
					}

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
