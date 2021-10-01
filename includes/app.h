#pragma once

#include <cstring>
#include <SDL.h>

#include "canvas.h"
#include "menu.h"
#include "event.h"

#include "renderer.h"
#include "window.h"

using namespace ctl;

class App
{
public:
	App()
	{
		m_w.init();
		m_r = m_w.create_renderer();

		m_canvas.init(m_r);
		m_menu.init(m_w, m_r);

		const mth::Dim<int> win = m_w.get_windowsize();

		tex.reset(
			SDL_CreateTexture(m_r._renderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, win.w, win.h));

		SDL_SetTextureBlendMode(tex.get(), SDL_BLENDMODE_BLEND);

		void *pixels;
		int	  pitch;

		SDL_LockTexture(tex.get(), nullptr, &pixels, &pitch);

		std::memset(pixels, 0, pitch / 4 * win.h);

		suf.reset(
			cairo_image_surface_create_for_data((unsigned char *)pixels, CAIRO_FORMAT_ARGB32, win.w, win.h, pitch));
		ctx.reset(cairo_create(suf.get()));

		// std::memset(pixels, 0, pitch / 4 * 300);

		cairo_set_source_rgb(ctx.get(), 0, 0, 0);
		cairo_set_line_width(ctx.get(), 10);
		cairo_set_line_cap(ctx.get(), CAIRO_LINE_CAP_ROUND);

		cairo_move_to(ctx.get(), 128.0, 50.0);
		cairo_line_to(ctx.get(), 128.0, 200.0);
		cairo_stroke(ctx.get());

		cairo_set_line_cap(ctx.get(), CAIRO_LINE_CAP_SQUARE);

		cairo_move_to(ctx.get(), 192.0, 50.0);
		cairo_line_to(ctx.get(), 192.0, 200.0);
		cairo_stroke(ctx.get());

		ctl::print("%ld\n", (long)pixels);

		SDL_UnlockTexture(tex.get());

		SDL_LockTexture(tex.get(), nullptr, &pixels, &pitch);

		/* draw helping lines */
		cairo_set_source_rgb(ctx.get(), 1, 0.2, 0.2);
		cairo_set_line_width(ctx.get(), 1);
		cairo_move_to(ctx.get(), 64.0, 50.0);
		cairo_line_to(ctx.get(), 64.0, 200.0);
		cairo_move_to(ctx.get(), 128.0, 50.0);
		cairo_line_to(ctx.get(), 128.0, 200.0);
		cairo_move_to(ctx.get(), 192.0, 50.0);
		cairo_line_to(ctx.get(), 192.0, 200.0);
		cairo_stroke(ctx.get());

		ctl::print("%ld\n", (long)pixels);

		SDL_UnlockTexture(tex.get());
	}

	void pre_pass()
	{
	}

	void event(const SDL_Event &e)
	{
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: m_ek.set(KeyEventMap::MOUSE_LEFT, true); break;
			case SDL_BUTTON_RIGHT: m_ek.set(KeyEventMap::MOUSE_RIGHT, true); break;
			case SDL_BUTTON_MIDDLE: m_ek.set(KeyEventMap::MOUSE_MIDDLE, true); break;
			}

			break;

		case SDL_MOUSEBUTTONUP:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: m_ek.set(KeyEventMap::MOUSE_LEFT, false); break;
			case SDL_BUTTON_RIGHT: m_ek.set(KeyEventMap::MOUSE_RIGHT, false); break;
			case SDL_BUTTON_MIDDLE: m_ek.set(KeyEventMap::MOUSE_MIDDLE, false); break;
			}

			break;

		case SDL_WINDOWEVENT: m_r.refresh(); break;
		}

		m_menu.event(e, m_w, m_r) && m_canvas.event(e, m_ek, m_w, m_r);
	}

	void update()
	{
	}

	void render()
	{
		m_r.render(
			[this]
			{
				// 				void *a;
				// 				int	  b;
				//
				// 				SDL_LockTexture(tex.get(), nullptr, &a, &b);
				//
				// 				cairo_set_source_rgb(ctx.get(), 0, 0, 0);
				// 				cairo_set_line_width(ctx.get(), 10);
				// 				cairo_set_line_cap(ctx.get(), CAIRO_LINE_CAP_ROUND);
				//
				// 				cairo_move_to(ctx.get(), 64.0, 50.0);
				// 				cairo_line_to(ctx.get(), 64.0, 200.0);
				// 				cairo_stroke(ctx.get());
				//
				// 				SDL_UnlockTexture(tex.get());
				//
				// 				SDL_RenderCopy(m_r._renderer(), tex.get(), nullptr, nullptr);

				m_canvas.draw(m_r);
				m_menu.draw(m_w, m_r);

#ifndef NDEBUG
				m_r.refresh();
#endif
			});
	}

private:
	Window	 m_w;
	Renderer m_r;
	KeyEvent m_ek;

	Canvas m_canvas;
	Menu   m_menu;

	sdl::Texture tex;

	CairoContext ctx;
	CairoSurface suf;
};
