#pragma once

#include "CustomLibrary/Collider.h"

#include "line.h"
#include "texture.h"
#include "event.h"

using namespace ctl;

struct CanvasContext
{
	std::vector<mth::Point<int>> circle_pattern;

	sdl::Window *  w;
	sdl::Renderer *r;

	sdl::Camera2D cam;

	std::vector<Line<float>>	lines;
	std::vector<Texture<float>> textures;

	Line<float>	 target_line;
	Texture<int> target_texture;
};

void erase(CanvasContext *c, size_t i)
{
	assert((c->textures.size() == c->lines.size()) && "Textures and lines not same size.");
	assert(!(c->textures.empty() || c->lines.empty()) && "Textures or lines empty.");

	std::swap(c->textures[i], c->textures.back());
	std::swap(c->lines[i], c->lines.back());
	/*
	std::iter_swap(c->textures.begin() + i, c->textures.end() - 1);
	std::iter_swap(c->lines.begin() + i, c->lines.end() - 1);
	*/

	c->textures.erase(c->textures.end() - 1);
	c->lines.erase(c->lines.end() - 1);
}

auto empty_texture(const CanvasContext *c) -> Texture<int>
{
	int w, h;
	SDL_GetWindowSize(c->w->get(), &w, &h);
	return { .dim = { 0, 0, w, h }, .data = sdl::create_empty(c->r->get(), w, h) };
}

void zoom(CanvasContext *c, int factor, float scale)
{
	mth::Point<int> mouse_p;
	SDL_GetMouseState(&mouse_p.x, &mouse_p.y);
	c->cam.zoom(1.F + factor / scale, mouse_p);
}

void push_target(CanvasContext *c)
{
	c->textures.push_back(
		{ .dim = c->cam.screen_world(c->target_texture.dim), .data = std::move(c->target_texture.data) });

	std::vector<mth::Point<float>> points;
	points.reserve(c->target_line.points.size());
	for (auto p : c->target_line.points) points.emplace_back(c->cam.screen_world(p));

	c->lines.push_back({ c->target_line.radius, c->target_line.color, std::move(points) });
}

void draw_circle(CanvasContext *c, const mth::Point<int> mouse)
{
	std::vector<SDL_Point> buf(c->circle_pattern.size());
	std::transform(c->circle_pattern.begin(), c->circle_pattern.end(), buf.begin(), [&mouse](mth::Point<int> p) {
		return SDL_Point{ p.x + mouse.x, p.y + mouse.y };
	});
	SDL_RenderDrawPoints(c->r->get(), buf.data(), c->circle_pattern.size());
}

void draw_connecting_line(CanvasContext *c, const mth::Point<int> to)
{
	const auto &from = c->target_line.points.back();

	for (int i = -c->target_line.radius, end = c->target_line.radius; i < end; ++i)
	{
		SDL_RenderDrawLine(c->r->get(), from.x, from.y + i, to.x, to.y + i);
		SDL_RenderDrawLine(c->r->get(), from.x + i, from.y, to.x + i, to.y);
	}
}

void draw_stroke(CanvasContext *c, mth::Point<int> to)
{
	SDL_SetRenderTarget(c->r->get(), c->target_texture.data.get());
	SDL_SetRenderDrawColor(c->r->get(), c->target_line.color.r, c->target_line.color.g, c->target_line.color.b,
						   c->target_line.color.a);

	draw_circle(c, to);
	draw_connecting_line(c, to);

	SDL_SetRenderTarget(c->r->get(), nullptr);

	c->target_line.points.emplace_back(to);
}

void start_stroke(CanvasContext *c)
{
	c->target_texture = empty_texture(c);

	int x, y;
	SDL_GetMouseState(&x, &y);
	c->target_line.points.emplace_back(x, y);

	SDL_SetRenderTarget(c->r->get(), c->target_texture.data.get());
	SDL_SetRenderDrawColor(c->r->get(), c->target_line.color.r, c->target_line.color.g, c->target_line.color.b,
						   c->target_line.color.a);

	draw_circle(c, { x, y });

	SDL_SetRenderTarget(c->r->get(), nullptr);
}

auto find_intersections(const CanvasContext *c, const mth::Point<float> p) -> std::vector<size_t>
{
	std::vector<size_t> res;

	for (int i = c->textures.size() - 1; i >= 0; --i)
		if (mth::collision(p, c->textures[i].dim))
			res.emplace_back(i);

	res.erase(std::remove_if(
				  res.begin(), res.end(),
				  [&c, p](const auto i) {
					  return std::none_of(
						  c->lines[i].points.begin(), c->lines[i].points.end(), [&c, i, p](const mth::Point<float> &l) {
							  const mth::Rect box = { l.x, l.y, c->lines[i].radius * 2, c->lines[i].radius * 2 };
							  return mth::collision(p, box);
						  });
				  }),
			  res.end());

	return res;
}

auto shrink_to_fit(SDL_Renderer *r, const CanvasContext *c) -> Texture<float>
{
	SDL_FPoint min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	SDL_FPoint max = { 0.F, 0.F };

	for (const auto &p : c->target_line.points) // Find line range inside texture
	{
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
	}

	min.x -= c->target_line.radius;
	min.y -= c->target_line.radius;
	max.x += c->target_line.radius;
	max.y += c->target_line.radius;

	const auto w = max.x - min.x;
	const auto h = max.y - min.y;

	const mth::Rect rect = { static_cast<int>(min.x), static_cast<int>(min.y), static_cast<int>(w),
							 static_cast<int>(h) };

	return { .dim = { rect.x, rect.y, rect.w, rect.h }, .data = sdl::crop(r, c->target_texture.data, rect) };
}

auto regen_pattern(int r) noexcept { return mth::gen_circle_filled(r); }

class Canvas
{
public:
	explicit Canvas(sdl::Window *w, sdl::Renderer *r)
	{
		m_context.w = w;
		m_context.r = r;

		m_context.circle_pattern = regen_pattern(m_context.target_line.radius);
	}

	void draw()
	{
		for (const auto &t : m_context.textures)
		{
			const auto world = m_context.cam.world_screen(mth::Rect{ t.dim.x, t.dim.y, t.dim.w, t.dim.h });
			SDL_RenderCopy(m_context.r->get(), t.data.get(), nullptr, &sdl::to_rect(world));
		}

		if (!m_context.target_line.points.empty())
			SDL_RenderCopy(m_context.r->get(), m_context.target_texture.data.get(), nullptr,
						   &sdl::to_rect(m_context.target_texture.dim));
	}

	void event(const SDL_Event &e, const KeyEvent &ke)
	{
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: start_stroke(&m_context); break;
			}

			break;

		case SDL_MOUSEBUTTONUP:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: push_target(&m_context); break;
			}

			break;

		case SDL_MOUSEMOTION:
			if (ke.test(KeyEventMap::MOUSE_LEFT))
			{
				draw_stroke(&m_context, { e.motion.x, e.motion.y });
				trace_point(&m_context.target_line, { e.motion.x, e.motion.y });
			}

			else if (ke.test(KeyEventMap::MOUSE_RIGHT))
				for (size_t i :
					 find_intersections(&m_context, m_context.cam.screen_world(mth::Point{ e.motion.x, e.motion.y })))
					erase(&m_context, i);

			else if (ke.test(KeyEventMap::MOUSE_MIDDLE))
				m_context.cam.translate(-e.motion.xrel, -e.motion.yrel);

			break;

		case SDL_MOUSEWHEEL: zoom(&m_context, e.wheel.y, 10.f); break;

		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
			case SDLK_DOWN:
				m_context.target_line.radius =
					std::clamp(m_context.target_line.radius + (e.key.keysym.sym == SDLK_UP ? 1 : -1), 0, 10);

				std::clog << "Radius: " << +m_context.target_line.radius << std::endl;

				m_context.circle_pattern = regen_pattern(m_context.target_line.radius);

				break;

			case SDLK_r: m_context.target_line.color = sdl::RED; break;
			case SDLK_b:
				m_context.target_line.color = sdl::BLACK;
				break;
				// case SDLK_s: save("save.xml", m_lines); break;
				// case SDLK_l:
				//	m_lines = load("save.xml");
				//	std::clog << "Found " << m_lines.size() << " lines.\n";
				//	draw_lines(m_lines);
				//	break;
			}
		}
	}

private:
	CanvasContext m_context;
};