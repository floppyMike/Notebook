#include <SDL.h>
#include <CustomLibrary/SDL/All.h>

using namespace ctl;

class App
{
public:
	App()
	{
		m_win.reset(SDL_CreateWindow("Apollo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
									 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));
		if (!m_win)
			throw std::runtime_error(SDL_GetError());

		m_rend.reset(SDL_CreateRenderer(m_win.get(), -1, SDL_RENDERER_ACCELERATED));
		if (!m_rend)
			throw std::runtime_error(SDL_GetError());
	}

	void pre_pass() {}
	void event(const SDL_Event &e)
	{
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN: m_press = true; break;
		case SDL_MOUSEBUTTONUP: m_press = false; break;
		case SDL_MOUSEMOTION: if (m_press) m_dots.push_back({ e.motion.x, e.motion.y }); break;
		}
	}
	void update() {}
	void render()
	{
		// SDL_SetRenderDrawColor(m_rend.get(), sdl::DARK_GRAY.r, sdl::DARK_GRAY.g, sdl::DARK_GRAY.b, sdl::DARK_GRAY.a);
		SDL_SetRenderDrawColor(m_rend.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);

		// 	Clear window
		SDL_RenderClear(m_rend.get());

		SDL_SetRenderDrawColor(m_rend.get(), sdl::BLACK.r, sdl::BLACK.g, sdl::BLACK.b, sdl::BLACK.a);
		SDL_RenderDrawPoints(m_rend.get(), m_dots.data(), m_dots.size());

		// Render Buffer
		SDL_RenderPresent(m_rend.get());
	}

private:
	sdl::Window	  m_win;
	sdl::Renderer m_rend;

	std::vector<SDL_Point> m_dots;
	bool				   m_press = false;
};

auto main() -> int
{
	sdl::SDL s;
	App		 a;
	sdl::run(&a, 30);

	return 0;
}
