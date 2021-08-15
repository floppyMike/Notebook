#pragma once

#include <CustomLibrary/SDL/All.h>

#include "renderer.h"

using namespace ctl;

struct WindowContext
{
	sdl::Window win;
};

class Window
{
public:
	Window()
	{
		m_con.win.reset(SDL_CreateWindow("Notetaker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
										 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

		if (!m_con.win)
			throw std::runtime_error(SDL_GetError());
	}

	auto create_renderer() const -> Renderer
	{
		return Renderer(m_con.win.get());
	}

	auto get_windowsize() const -> mth::Dim<int>
	{
		mth::Dim<int> dim;
		SDL_GetWindowSize(m_con.win.get(), &dim.w, &dim.h);

		return dim;
	}

	auto get_mousepos() const -> mth::Point<int>
	{
		mth::Point<int> mouse_p;
		SDL_GetMouseState(&mouse_p.x, &mouse_p.y);

		return mouse_p;
	}

private:
	WindowContext m_con;
};
