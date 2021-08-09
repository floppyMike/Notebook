#pragma once

#include <CustomLibrary/SDL/All.h>

#include "comp/renderer.h"

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

	auto create_renderer() const -> Renderer { return Renderer(m_con.win.get()); }

private:
	WindowContext m_con;
};
