#pragma once

#include <CustomLibrary/SDL/All.h>

using namespace ctl;

struct RendererContext
{
	sdl::Renderer rend;
};

class Renderer
{
public:
	explicit Renderer(SDL_Window *win)
	{
		m_con.rend.reset(SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE));

		if (m_con.rend)
			throw std::runtime_error(SDL_GetError());
	}

	template<typename T>
	void render(T &&draws)
	{
		SDL_SetRenderDrawColor(m_con.rend.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);
		SDL_RenderClear(m_con.rend.get());

		draws();

		SDL_RenderPresent(m_con.rend.get());
	}

private:
	RendererContext m_con;
};
