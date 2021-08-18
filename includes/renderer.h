#pragma once

#include <CustomLibrary/SDL/All.h>

using namespace ctl;

using TextureData = sdl::Texture;

struct RendererContext
{
	sdl::Renderer r;
	bool		  refresh = true;

	std::vector<mth::Point<int>> circle_pattern;
};

class Renderer
{
public:
	explicit Renderer(SDL_Window *win)
	{
		m_con.r.reset(SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE));

		if (!m_con.r)
			throw std::runtime_error(SDL_GetError());

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Enable a blur effect when copying textures
	}

	void set_draw_color(SDL_Color col) const
	{
		SDL_SetRenderDrawColor(m_con.r.get(), col.r, col.g, col.b, col.a);
	}

	void set_target()
	{
		SDL_SetRenderTarget(m_con.r.get(), nullptr);
	}

	void set_target(const TextureData &t)
	{
		SDL_SetRenderTarget(m_con.r.get(), t.get());
	}

	void set_stroke_radius(int r)
	{
		m_con.circle_pattern = mth::gen_circle_filled(r);
	}

	void draw_conn_stroke(mth::Point<int> p, int r) const
	{
		assert(!m_con.circle_pattern.empty() && "Circle pattern not generated yet.");

		std::vector<SDL_Point> buf(m_con.circle_pattern.size());
		std::transform(m_con.circle_pattern.begin(), m_con.circle_pattern.end(), buf.begin(),
					   [m = p](mth::Point<int> p) {
						   return SDL_Point{ p.x + m.x, p.y + m.y };
					   });

		SDL_RenderDrawPoints(m_con.r.get(), buf.data(), (int)m_con.circle_pattern.size());
	}

	void render_conn_stroke(mth::Point<int> p, int r) const
	{
		draw_conn_stroke(p, r);
		SDL_RenderPresent(m_con.r.get());
	}

	void draw_inter_stroke(mth::Point<int> from, mth::Point<int> to, int r) const
	{
		for (int i = -(r - 1); i <= r - 1; ++i)
		{
			SDL_RenderDrawLine(m_con.r.get(), from.x, from.y + i, to.x, to.y + i);
			SDL_RenderDrawLine(m_con.r.get(), from.x + i, from.y, to.x + i, to.y);
		}
	}

	void render_inter_stroke(mth::Point<int> from, mth::Point<int> to, int r) const
	{
		draw_inter_stroke(from, to, r);
		SDL_RenderPresent(m_con.r.get());
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

	void draw_rect(mth::Rect<int, int> r) const
	{
		SDL_RenderDrawRect(m_con.r.get(), &sdl::to_rect(r));
	}

	void render_rect(mth::Rect<int, int> r) const
	{
		draw_rect(r);
		SDL_RenderPresent(m_con.r.get());
	}

	void refresh()
	{
		m_con.refresh = true;
	}

	template<typename T>
	void render(T &&draws)
	{
		if (!m_con.refresh)
			return;

		SDL_SetRenderTarget(m_con.r.get(), nullptr);

		SDL_SetRenderDrawColor(m_con.r.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);
		SDL_RenderClear(m_con.r.get());

		draws();

		SDL_RenderPresent(m_con.r.get());

		m_con.refresh = false;
	}

private:
	RendererContext m_con;
};
