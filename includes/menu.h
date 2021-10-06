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
		auto bmp = r.load_bmp("res/Icons.bmp");

		if (!bmp)
			throw std::runtime_error("File res/Icons.bmp not found.");

		c.icon_map = std::move(*bmp);

		c.icons = {
			{ .id = EVENT_SELECT }, { .id = EVENT_DRAW }, { .id = EVENT_TYPE },
			{ .id = EVENT_SAVE },	{ .id = EVENT_LOAD },
		};

		c.bar = gen_bar(r, c.icon_map, c.icons, 0);

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
		case EVENT_TYPE:
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
						SDL_Event event = sdl::create_event(e.button.windowID, f->id);
						SDL_PushEvent(&event);

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
