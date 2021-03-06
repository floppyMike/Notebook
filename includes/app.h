#pragma once

#include <cstring>
#include <SDL.h>

#include "CustomLibrary/Error.h"
#include "canvas.h"
#include "menu.h"
#include "event.h"

#include "renderer.h"
#include "window.h"

using namespace ctl;

class App
{
public:
	App()
	{
		m_w.init();
		m_r = m_w.create_renderer();

		m_canvas.init(m_r);
		m_menu.init(m_w, m_r);

		sdl::push_event(m_w.get_windowid(), EVENT_DRAW);
	}

	void pre_pass()
	{
	}

	void event(const SDL_Event &e)
	{
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: m_ek.set(KeyEventMap::MOUSE_LEFT, true); break;
			case SDL_BUTTON_RIGHT: m_ek.set(KeyEventMap::MOUSE_RIGHT, true); break;
			case SDL_BUTTON_MIDDLE: m_ek.set(KeyEventMap::MOUSE_MIDDLE, true); break;
			}

			break;

		case SDL_MOUSEBUTTONUP:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: m_ek.set(KeyEventMap::MOUSE_LEFT, false); break;
			case SDL_BUTTON_RIGHT: m_ek.set(KeyEventMap::MOUSE_RIGHT, false); break;
			case SDL_BUTTON_MIDDLE: m_ek.set(KeyEventMap::MOUSE_MIDDLE, false); break;
			}

			break;

		case SDL_WINDOWEVENT: m_r.refresh(); break;
		}

		m_menu.event(e, m_w, m_r) && m_canvas.event(e, m_ek, m_w, m_r);
	}

	void update()
	{
	}

	void render()
	{
		m_r.render(
			[this]
			{
				m_canvas.draw(m_r);
				m_menu.draw(m_w, m_r);

#ifndef NDEBUG
				m_r.refresh();
#endif
			});
	}

private:
	Window	 m_w;
	Renderer m_r;
	KeyEvent m_ek;

	Canvas m_canvas;
	Menu   m_menu;
};
