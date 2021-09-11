#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include <CustomLibrary/SDL/All.h>

using namespace ctl;

using TextureData = sdl::Texture;
using FontData	  = sdl::Font;

struct RendererContext
{
	sdl::Renderer r;
	bool		  refresh = true;

	std::vector<mth::Point<int>> circle_pattern;
};

inline auto translate_circle(const RendererContext &c, mth::Point<int> m) -> std::vector<SDL_Point>
{
	std::vector<SDL_Point> buf(c.circle_pattern.size());
	std::transform(c.circle_pattern.begin(), c.circle_pattern.end(), buf.begin(),
				   [m](mth::Point<int> p) {
					   return SDL_Point{ p.x + m.x, p.y + m.y };
				   });

	return buf;
}

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

	void set_render_target(const TextureData &t)
	{
		SDL_SetRenderTarget(m_con.r.get(), t.get());
	}

	auto get_texture_size(const TextureData &t)
	{
		mth::Dim<int> d;
		SDL_QueryTexture(t.get(), nullptr, nullptr, &d.w, &d.h);

		return d;
	}

	void render_target()
	{
		SDL_RenderPresent(m_con.r.get());
	}

	void set_stroke_radius(int r)
	{
		m_con.circle_pattern.clear();

		for (auto x = -r; x <= +r; ++x)
			for (auto y = -r; y <= +r; ++y)
				if (y * y + x * x < r * r)
					m_con.circle_pattern.push_back({ x, y });
	}

	void draw_joint_stroke(mth::Point<int> p, int r) const
	{
		assert(!m_con.circle_pattern.empty() && "Circle pattern not generated yet.");

		const auto buf = translate_circle(m_con, p);
		SDL_RenderDrawPoints(m_con.r.get(), buf.data(), (int)m_con.circle_pattern.size());
	}

	void draw_con_stroke(mth::Point<int> from, mth::Point<int> to, int r) const
	{
		for (int i = -(r - 1); i <= r - 1; ++i)
		{
			SDL_RenderDrawLine(m_con.r.get(), from.x, from.y + i, to.x, to.y + i);
			SDL_RenderDrawLine(m_con.r.get(), from.x + i, from.y, to.x + i, to.y);
		}
	}

	auto create_texture(int w, int h) const
	{
		return sdl::create_empty(m_con.r.get(), w, h);
	}

	auto create_font(const char *path, int size)
	{
		return sdl::load_font(path, size);
	}

	auto create_text(const FontData &f, const char *text) const
	{
		auto *s = TTF_RenderText_Blended_Wrapped(f.get(), text, sdl::BLACK, 600);

		if (s == nullptr)
			throw std::runtime_error(TTF_GetError());

		auto t = TextureData(SDL_CreateTextureFromSurface(m_con.r.get(), s));

		SDL_FreeSurface(s);

		return t;
	}

	auto crop_texture(const TextureData &t, const mth::Rect<int> &r) const
	{
		return sdl::crop(m_con.r.get(), t, r);
	}

	void draw_texture(const TextureData &t, const mth::Rect<int> &r) const
	{
		SDL_RenderCopy(m_con.r.get(), t.get(), nullptr, &sdl::to_rect(r));
	}

	void draw_rect(mth::Rect<int> r) const
	{
		SDL_RenderDrawRect(m_con.r.get(), &sdl::to_rect(r));
	}

	void draw_line(mth::Point<int> start, mth::Point<int> end) const
	{
		SDL_RenderDrawLine(m_con.r.get(), start.x, start.y, end.x, end.y);
	}

	void refresh()
	{
		m_con.refresh = true;
	}

	template<typename T>
	requires std::is_invocable_v<T>
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
