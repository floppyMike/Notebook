#pragma once

#include <CustomLibrary/SDL/All.h>

using namespace ctl;

using TextureData = sdl::Texture;

struct RendererContext
{
	sdl::Renderer				 r;
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
		m_con.r.reset(SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE));

		if (!m_con.r)
			throw std::runtime_error(SDL_GetError());
	}

	auto create_texture(int w, int h) const
	{
		return sdl::create_empty(m_con.r.get(), w, h);
	}

	auto crop_texture(const TextureData &t, const mth::Rect<int, int> &r) const
	{
		return sdl::crop(m_con.r.get(), t, r);
	}

	auto draw_texture(const TextureData &t, const mth::Rect<int, int> &r) const
	{
		SDL_RenderCopy(m_con.r.get(), t.get(), nullptr, &sdl::to_rect(r));
	}

	void set_draw_color(SDL_Color col) const
	{
		SDL_SetRenderDrawColor(m_con.r.get(), col.r, col.g, col.b, col.a);
	}

	void draw_rect(mth::Rect<int, int> r) const
	{
		SDL_RenderDrawRect(m_con.r.get(), &sdl::to_rect(r));
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

	template<typename T>
	void render(T &&draws)
	{
		SDL_SetRenderDrawColor(m_con.r.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);
		SDL_RenderClear(m_con.r.get());

		draws();

		SDL_RenderPresent(m_con.r.get());
	}

private:
	RendererContext m_con;
};
