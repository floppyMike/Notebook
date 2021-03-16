#include <SDL.h>
#include <CustomLibrary/SDL/All.h>

#include <CustomLibrary/Error.h>

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
			if (e.button.button != SDL_BUTTON_LEFT)
				break;

			m_press = true;
			ASSERT(m_dots = sdl::Texture(
					   SDL_CreateTexture(m_rend.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 640, 480)),
				   "Texture dot creation failed.");

			SDL_SetTextureBlendMode(m_dots.get(), SDL_BLENDMODE_BLEND);

			SDL_SetRenderTarget(m_rend.get(), m_dots.get());
			SDL_SetRenderDrawColor(m_rend.get(), 0, 0, 0, 0);
			SDL_RenderClear(m_rend.get());
			SDL_SetRenderTarget(m_rend.get(), nullptr);

			break;

		case SDL_MOUSEBUTTONUP:
			m_press = false;
			m_prev_dots.emplace_back(std::move(m_dots));
			break;

		case SDL_MOUSEMOTION:
			if (m_press)
			{
				SDL_SetRenderTarget(m_rend.get(), m_dots.get());

				SDL_SetRenderDrawColor(m_rend.get(), sdl::BLACK.r, sdl::BLACK.g, sdl::BLACK.b, sdl::BLACK.a);
				const SDL_Rect dest = { e.motion.x - (m_dim >> 1), e.motion.y - (m_dim >> 1), m_dim, m_dim };
				SDL_RenderFillRect(m_rend.get(), &dest);

				SDL_SetRenderTarget(m_rend.get(), nullptr);
			}
			break;

		case SDL_MOUSEWHEEL: m_dim += e.wheel.y; break;
		}
	}
	void update() {}
	void render()
	{
		SDL_SetRenderDrawColor(m_rend.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);

		// 	Clear window
		SDL_RenderClear(m_rend.get());

		for (const sdl::Texture &t : m_prev_dots) SDL_RenderCopy(m_rend.get(), t.get(), nullptr, nullptr);

		if (m_dots)
			SDL_RenderCopy(m_rend.get(), m_dots.get(), nullptr, nullptr);

		// Render Buffer
		SDL_RenderPresent(m_rend.get());
	}

private:
	sdl::Window	  m_win;
	sdl::Renderer m_rend;

	sdl::Texture			  m_dots;
	bool					  m_press = false;
	std::vector<sdl::Texture> m_prev_dots;

	int m_dim = 8;
};

auto main() -> int
{
	sdl::SDL s;
	App		 a;
	sdl::run(&a, 61);

	return 0;
}
