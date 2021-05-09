#pragma once

#include <SDL.h>
#include <CustomLibrary/SDL/All.h>
#include <CustomLibrary/Error.h>
#include <CustomLibrary/Collider.h>

#include "canvas.h"
#include "event.h"

using namespace ctl;

class App
{
public:
	App()
		: m_canvas(&m_win, &m_rend)
	{
		m_win.reset(SDL_CreateWindow("Notetaker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
									 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));
		if (!m_win)
			throw std::runtime_error(SDL_GetError());

		m_rend.reset(SDL_CreateRenderer(m_win.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE));
		if (!m_rend)
			throw std::runtime_error(SDL_GetError());
	}

	void pre_pass() {}
	void event(const SDL_Event &e)
	{
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

		m_canvas.event(e, m_events);
	}

	void update() {}
	void render()
	{
		SDL_SetRenderDrawColor(m_rend.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);
		SDL_RenderClear(m_rend.get());

		m_canvas.draw();

		SDL_RenderPresent(m_rend.get());
	}

private:
	sdl::Window	  m_win;
	sdl::Renderer m_rend;

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
