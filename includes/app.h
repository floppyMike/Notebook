#pragma once

#include <SDL.h>
#include <CustomLibrary/SDL/All.h>
#include <CustomLibrary/Error.h>
#include <CustomLibrary/Collider.h>

#include "canvas.h"
#include "event.h"
#include "pipe.h"

#include "comp/renderer.h"
#include "comp/window.h"

using namespace ctl;

struct AppContext
{
	Window	 win;
	Renderer rend;
	KeyEvent events;

	Canvas canvas;
};

class App
{
public:
	App()
		: m_win()
		, m_rend(m_win.create_renderer())
		, m_events()
	{
	}

	void pre_pass()
	{
	}

	void event(const SDL_Event &e)
	{
		// Assign to map
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: m_events.set(KeyEventMap::MOUSE_LEFT, true); break;
			case SDL_BUTTON_RIGHT: m_events.set(KeyEventMap::MOUSE_RIGHT, true); break;
			case SDL_BUTTON_MIDDLE: m_events.set(KeyEventMap::MOUSE_MIDDLE, true); break;
			}

			break;

		case SDL_MOUSEBUTTONUP:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: m_events.set(KeyEventMap::MOUSE_LEFT, false); break;
			case SDL_BUTTON_RIGHT: m_events.set(KeyEventMap::MOUSE_RIGHT, false); break;
			case SDL_BUTTON_MIDDLE: m_events.set(KeyEventMap::MOUSE_MIDDLE, false); break;
			}

			break;
		}

		// Delegate to components
		EventPipe ep = { .e = e, .ke = m_events, .w = &m_win, .r = &m_rend };

		m_canvas.event(ep);
	}

	void update()
	{
	}
	void render()
	{
		m_rend.render(
			[this]
			{
				m_rend.set_draw_color(0, 0, 0, 0xFF);
				m_rend.draw_rect({ 100, 100, 100, 100 });

				m_canvas.draw(&m_rend);
			});
	}

private:
	// AppContext m_con;

	Window	 m_win;
	Renderer m_rend;
	KeyEvent m_events;

	Canvas m_canvas;
};

/*
void App::draw_lines(const std::vector<Line<float>> &ls)
{
	SDL_SetRenderTarget(m_rend.get(), nullptr);

	for (auto &l : ls)
	{
		SDL_SetRenderDrawColor(m_rend.get(), l.color.r, l.color.g, l.color.b, l.color.a);

		draw_circle(l.points.front());

		for (auto b = l.points.begin() + 1; b != l.points.end(); ++b)
		{
			draw_connecting_line(*(b - 1), *b);
			draw_circle(*b);
		}
	}

	SDL_SetRenderTarget(m_rend.get(), nullptr);
}
*/
