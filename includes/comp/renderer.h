#pragma once

#include <CustomLibrary/SDL/All.h>

using namespace ctl;

using TextureData = sdl::Texture;

struct RendererContext
{
	sdl::Renderer rend;
	std::vector<mth::Point<int>> circle_pattern;
};

// -----------------------------------------------------------------------------
// Renderer Interface
// -----------------------------------------------------------------------------

class Renderer
{
public:
	explicit Renderer(SDL_Window *win)
	{
		m_con.rend.reset(SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE));

		if (!m_con.rend)
			throw std::runtime_error(SDL_GetError());
	}

	// --------------------------------- Texture Manip -----------------------------------------

	auto create_texture(int w, int h) const
	{
		return sdl::create_empty(m_con.rend.get(), w, h);
	}

	auto crop_texture(const TextureData &t, const mth::Rect<int, int> &r) const
	{
		return sdl::crop(m_con.rend.get(), t, r);
	}

	auto draw_texture(const TextureData &t, const mth::Rect<int, int> &r) const
	{
		SDL_RenderCopy(m_con.rend.get(), t.get(), nullptr, &sdl::to_rect(r));
	}

	// --------------------------------- Primitive drawing -----------------------------------------

	void set_draw_color(int r, int g, int b, int a) const
	{
		SDL_SetRenderDrawColor(m_con.rend.get(), r, g, b, a);
	}

	void draw_rect(mth::Rect<int, int> r) const
	{
		SDL_RenderDrawRect(m_con.rend.get(), &sdl::to_rect(r));
	}

// 	void draw_filled_circle(const mth::Point<int> mouse)
// 	{
// 		std::vector<SDL_Point> buf(c->circle_pattern.size());
// 		std::transform(c->circle_pattern.begin(), c->circle_pattern.end(), buf.begin(),
// 					   [&mouse](mth::Point<int> p) {
// 						   return SDL_Point{ p.x + mouse.x, p.y + mouse.y };
// 					   });
// 		SDL_RenderDrawPoints(c->r->get(), buf.data(), c->circle_pattern.size());
// 	}

// 	void draw_line(const mth::Point<int> from, const mth::Point<int> to)
// 	{
// 		for (int i = -c->target_line.radius, end = c->target_line.radius; i < end; ++i)
// 		{
// 			SDL_RenderDrawLine(c->r->get(), from.x, from.y + i, to.x, to.y + i);
// 			SDL_RenderDrawLine(c->r->get(), from.x + i, from.y, to.x + i, to.y);
// 		}
// 	}

	// --------------------------------- Render -----------------------------------------

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
