#pragma once

#include <cstring>
#include <span>

#include <SDL.h>
#include <SDL_ttf.h>

#include <cairo.h>
#include <CustomLibrary/SDL/All.h>
#include <CustomLibrary/Error.h>

using namespace ctl;

struct _CairoContextDeleter
{
	void operator()(cairo_t *c)
	{
		cairo_destroy(c);
	}
};

struct _CairoSurfaceDeleter
{
	void operator()(cairo_surface_t *s)
	{
		cairo_surface_destroy(s);
	}
};

using CairoContext = std::unique_ptr<cairo_t, _CairoContextDeleter>;
using CairoSurface = std::unique_ptr<cairo_surface_t, _CairoSurfaceDeleter>;

struct RendererContext
{
	sdl::Renderer r;
	bool		  refresh = true;

	CairoContext cxt;
	CairoSurface surf;
};

class Renderer
{
public:
	auto _renderer() const
	{
		return c.r.get();
	}

	using CacheTexture = sdl::Texture;
	using Texture	   = sdl::Texture;
	using Font		   = sdl::Font;

	void init(SDL_Window *win)
	{
		c.r.reset(SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE));

		if (!c.r)
			throw std::runtime_error(SDL_GetError());

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Enable a blur effect when copying textures
	}

	void refresh()
	{
		c.refresh = true;
	}

	template<typename T>
	requires std::is_invocable_v<T>
	void render(T &&draws)
	{
		if (!c.refresh)
			return;

		c.refresh = false;

		SDL_SetRenderTarget(c.r.get(), nullptr);

		SDL_SetRenderDrawColor(c.r.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);
		SDL_RenderClear(c.r.get());

		draws();

		SDL_RenderPresent(c.r.get());
	}

	void set_render_target(const Texture &t)
	{
		ASSERT(SDL_SetRenderTarget(c.r.get(), t.get()) == 0, SDL_GetError());
	}

	auto get_texture_size(const Texture &t) const
	{
		mth::Dim<int> d;
		ASSERT(SDL_QueryTexture(t.get(), nullptr, nullptr, &d.w, &d.h) == 0, SDL_GetError());

		return d;
	}

	void render_target()
	{
		SDL_RenderPresent(c.r.get());
	}

	auto create_texture(int w, int h) const
	{
		return sdl::create_empty_texture(c.r.get(), w, h);
	}

	auto create_font(const char *path, int size)
	{
		return sdl::load_font(path, size);
	}

	auto create_text(const Font &f, std::string_view text) const
	{
		if (text == "") // Avoid crash on empty string
			text = " ";

		sdl::Surface s(TTF_RenderText_Blended_Wrapped(f.get(), text.data(), sdl::BLACK, 600));
		ASSERT(s != nullptr, TTF_GetError());

		return Texture(SDL_CreateTextureFromSurface(c.r.get(), s.get()));
	}

	auto load_bmp(const char *path) -> std::optional<Texture>
	{
		sdl::Surface s(SDL_LoadBMP(path));
		return s ? std::optional(Texture(SDL_CreateTextureFromSurface(c.r.get(), s.get()))) : std::nullopt;
	}

	auto crop_texture(const Texture &t, mth::Rect<int> r) const
	{
		return sdl::crop(c.r.get(), t, r);
	}

	void draw_texture(const Texture &t, mth::Rect<int> r) const
	{
		ASSERT(SDL_RenderCopy(c.r.get(), t.get(), nullptr, &sdl::to_rect(r)) == 0, SDL_GetError());
	}

	void draw_frame(const Texture &t, mth::Rect<int> source, mth::Rect<int> dest) const
	{
		ASSERT(SDL_RenderCopy(c.r.get(), t.get(), &sdl::to_rect(source), &sdl::to_rect(dest)) == 0, SDL_GetError());
	}

	// -----------------------------------------------------------------------------
	// Primitive rendering
	// -----------------------------------------------------------------------------

	void set_draw_color(SDL_Color col) const
	{
		ASSERT(SDL_SetRenderDrawColor(c.r.get(), col.r, col.g, col.b, col.a) == 0, SDL_GetError());
	}

	void draw_rect(mth::Rect<int> r) const
	{
		ASSERT(SDL_RenderDrawRect(c.r.get(), &sdl::to_rect(r)) == 0, SDL_GetError());
	}

	void draw_rectfilled(mth::Rect<int> r) const
	{
		ASSERT(SDL_RenderFillRect(c.r.get(), &sdl::to_rect(r)) == 0, SDL_GetError());
	}

	void draw_line(mth::Point<int> start, mth::Point<int> end) const
	{
		ASSERT(SDL_RenderDrawLine(c.r.get(), start.x, start.y, end.x, end.y) == 0, SDL_GetError());
	}

	// -----------------------------------------------------------------------------
	// Stroke manip
	// -----------------------------------------------------------------------------

	auto create_stroke_texture(int w, int h) -> CacheTexture
	{
		CacheTexture t(SDL_CreateTexture(c.r.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h));

		SDL_SetTextureBlendMode(t.get(), SDL_BLENDMODE_BLEND);

		void *pixels;
		int	  pitch;
		SDL_LockTexture(t.get(), nullptr, &pixels, &pitch);

		std::memset(pixels, 0, pitch / 4 * h); // Make texture transparent (pitch is rgba * width)

		// God, please let the address stay the same
		c.surf.reset(cairo_image_surface_create_for_data((unsigned char *)pixels, CAIRO_FORMAT_ARGB32, w, h, pitch));
		c.cxt.reset(cairo_create(c.surf.get()));

		return t;
	}

	void set_stroke_color(SDL_Color col)
	{
		assert(c.cxt);
		cairo_set_source_rgba(c.cxt.get(), col.r / 255., col.g / 255., col.b / 255., col.a / 255.);
	}

	void set_stroke_target(const CacheTexture &t, mth::Rect<int> area, float r)
	{
		assert(t && c.cxt);

		void *pixels;
		int	  pitch;

		SDL_LockTexture(t.get(), &sdl::to_rect(area), &pixels, &pitch);

		cairo_set_line_width(c.cxt.get(), r);
		cairo_set_line_cap(c.cxt.get(), CAIRO_LINE_CAP_ROUND);
	}

	void render_stroke(const CacheTexture &t)
	{
		assert(t);
		SDL_UnlockTexture(t.get()); // Super slow, but I have to :(
	}

	void draw_stroke(mth::Point<int> from, mth::Point<int> to) const
	{
		assert(c.cxt);

		cairo_move_to(c.cxt.get(), (double)from.x, (double)from.y);
		cairo_line_to(c.cxt.get(), (double)to.x, (double)to.y);

		cairo_stroke(c.cxt.get());
	}

	void draw_stroke_multi(std::span<mth::Point<int>> arr) const
	{
		assert(c.cxt);

		if (arr.size() <= 1)
			return;

		if (arr.size() <= 2)
		{
			draw_stroke(arr[0], arr[1]);
			return;
		}

		cairo_move_to(c.cxt.get(), (double)arr[0].x, (double)arr[0].y);

		for (auto i = arr.begin() + 1; i != arr.end(); ++i) cairo_line_to(c.cxt.get(), (double)i->x, (double)i->y);

		cairo_stroke(c.cxt.get());
	}

private:
	RendererContext c;
};
