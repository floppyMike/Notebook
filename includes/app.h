#pragma once

#include <SDL.h>
#include <CustomLibrary/SDL/All.h>
#include <CustomLibrary/Error.h>

#include "line.h"
#include "texture.h"

using namespace ctl;

auto generate_draw_circle(const int r) noexcept { return mth::gen_circle_filled(r); }

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

		m_circle_pattern = generate_draw_circle(m_target_line.radius);
	}

	void pre_pass() {}
	void event(const SDL_Event &e)
	{
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT:
				m_press_left	 = true;
				m_target_texture = empty_texture();
				break;
			case SDL_BUTTON_RIGHT: m_press_right = true; break;
			case SDL_BUTTON_MIDDLE: m_press_middle = true; break;
			}

			break;

		case SDL_MOUSEBUTTONUP:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT:
				m_press_left = false;

				add_texture(std::move(m_target_texture), m_target_line);
				add_line(m_target_line);

				m_target_line.points.clear();

				break;

			case SDL_BUTTON_RIGHT: m_press_right = false; break;
			case SDL_BUTTON_MIDDLE: m_press_middle = false; break;
			}

			break;

		case SDL_MOUSEMOTION:
			if (m_press_left)
			{
				draw_line_to({ e.motion.x - e.motion.xrel, e.motion.y - e.motion.yrel }, { e.motion.x, e.motion.y });
				trace_point({ e.motion.x, e.motion.y });
			}
			else if (m_press_right)
				for (size_t i : find_intersections({ e.motion.x, e.motion.y })) delete_lines(i);
			else if (m_press_middle)
				m_cam.translate(-e.motion.xrel, -e.motion.yrel);

			break;

		case SDL_KEYDOWN:
			if (e.key.keysym.sym == SDLK_UP)
				m_target_line.radius += 1;
			else if (e.key.keysym.sym == SDLK_DOWN)
				m_target_line.radius -= 1;

			std::clog << "Radius: " << +m_target_line.radius << std::endl;
			m_circle_pattern = generate_draw_circle(m_target_line.radius);

			break;

		case SDL_MOUSEWHEEL: zoom(e.wheel.y, 10.f); break;
		}
	}
	void update() {}
	void render()
	{
		SDL_SetRenderDrawColor(m_rend.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);

		SDL_RenderClear(m_rend.get());

		for (const auto &t : m_textures)
		{
			const auto world = m_cam.world_screen(mth::Rect{ t.dim.x, t.dim.y, t.dim.w, t.dim.h });
			SDL_RenderCopy(m_rend.get(), t.data.get(), nullptr, &sdl::to_rect(world));
		}

		if (!m_target_line.points.empty())
		{
			SDL_RenderCopy(m_rend.get(), m_target_texture.data.get(), nullptr, &sdl::to_rect(m_target_texture.dim));
		}

		SDL_RenderPresent(m_rend.get());
	}

private:
	sdl::Window	  m_win;
	sdl::Renderer m_rend;

	Texture<int> m_target_texture;
	Line<int>	 m_target_line;

	bool m_press_left = false, m_press_right = false, m_press_middle = false;

	sdl::Camera2D				m_cam;
	std::vector<Texture<float>> m_textures;
	std::vector<Line<float>>	m_lines;

	std::vector<mth::Point<int>> m_circle_pattern;

	void delete_lines(size_t i);
	void draw_circle(mth::Point<int> mouse) const;
	void draw_connecting_line(mth::Point<int> from, mth::Point<int> to) const;
	auto find_intersections(mth::Point<int> p) const -> std::vector<size_t>;
	auto empty_texture() const -> Texture<int>;
	void add_texture(Texture<int> &&t, const Line<int> &ref);
	void add_line(const Line<int> &l);
	void draw_line_to(mth::Point<int> from, mth::Point<int> to);
	void trace_point(mth::Point<int> to);
	void zoom(int factor, float scale);
};

void App::zoom(int factor, float scale)
{
	mth::Point<int> mouse_p;
	SDL_GetMouseState(&mouse_p.x, &mouse_p.y);
	m_cam.zoom(1.F + factor / scale, mouse_p);
}

void App::trace_point(mth::Point<int> to)
{
	m_target_line.points.emplace_back(to.x, to.y);
}

void App::draw_line_to(mth::Point<int> from, mth::Point<int> to)
{
	SDL_SetRenderTarget(m_rend.get(), m_target_texture.data.get());
	SDL_SetRenderDrawColor(m_rend.get(), sdl::BLACK.r, sdl::BLACK.g, sdl::BLACK.b, sdl::BLACK.a);

	draw_circle(to);
	draw_connecting_line(from, to);

	SDL_SetRenderTarget(m_rend.get(), nullptr);
}

void App::add_texture(Texture<int> &&tex, const Line<int> &ref)
{
	auto t = shrink_to_fit(m_rend.get(), ref, tex.data); // Shrink texture

	m_textures.push_back({ .dim	 = { m_cam.screen_world(mth::Point<float>{ t.dim.x, t.dim.y }),
									 m_cam.screen_world(mth::Dim<float>{ t.dim.w, t.dim.h }) },
						   .data = std::move(t.data) });
}

void App::add_line(const Line<int> &l)
{
	std::vector<mth::Point<float>> points;
	points.reserve(l.points.size());

	for (auto p : l.points) points.emplace_back(m_cam.screen_world(p));

	m_lines.push_back({ l.radius, std::move(points) });
}

auto App::empty_texture() const -> Texture<int>
{
	int w, h;
	SDL_GetWindowSize(m_win.get(), &w, &h);
	return { .dim = { 0, 0, w, h }, .data = create_empty(m_rend.get(), w, h) };
}

void App::delete_lines(size_t i)
{
	std::swap(m_textures[i], m_textures.back());
	std::swap(m_lines[i], m_lines.back());
	m_textures.erase(m_textures.end() - 1);
	m_lines.erase(m_lines.end() - 1);
}

void App::draw_circle(const mth::Point<int> mouse) const
{
	std::vector<SDL_Point> buf(m_circle_pattern.size());
	std::transform(m_circle_pattern.begin(), m_circle_pattern.end(), buf.begin(), [&mouse](mth::Point<int> p) {
		return SDL_Point{ p.x + mouse.x, p.y + mouse.y };
	});
	SDL_RenderDrawPoints(m_rend.get(), buf.data(), m_circle_pattern.size());
}

void App::draw_connecting_line(const mth::Point<int> from, const mth::Point<int> to) const
{
	for (int i = -m_target_line.radius, end = m_target_line.radius; i < end; ++i)
	{
		SDL_RenderDrawLine(m_rend.get(), from.x, from.y + i, to.x, to.y + i);
		SDL_RenderDrawLine(m_rend.get(), from.x + i, from.y, to.x + i, to.y);
	}
}

auto App::find_intersections(const mth::Point<int> p) const -> std::vector<size_t>
{
	std::vector<size_t> res;

	// for (size_t i = 0; i < m_textures.size(); ++i)
	// 	if (SDL_PointInRect(&p, &m_textures[i].dim))
	// 		res.emplace_back(i);

	/*
	res.erase(std::remove_if(
				  res.begin(), res.end(),
				  [this, p](const auto i) {
					  return std::none_of(
						  m_lines[i].points.begin(), m_lines[i].points.end(), [this, i, p](const SDL_Point &l) {
							  const SDL_Rect box = { l.x, l.y, m_lines[i].radius * 2, m_lines[i].radius * 2 };
							  return SDL_PointInRect(&p, &box);
						  });
				  }),
			  res.end());
			  */

	return res;
}
