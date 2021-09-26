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
	void init()
	{
		m_con.win.reset(SDL_CreateWindow("Notetaker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
										 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

		if (!m_con.win)
			throw std::runtime_error(SDL_GetError());
	}

	auto create_renderer() const -> Renderer
	{
		Renderer r;
		r.init(m_con.win.get());
		return r;
	}

	auto get_windowsize() const -> mth::Dim<int>
	{
		mth::Dim<int> dim;
		SDL_GetWindowSize(m_con.win.get(), &dim.w, &dim.h);

		return dim;
	}

private:
	WindowContext m_con;
};
