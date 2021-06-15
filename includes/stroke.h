#pragma once

#include <CustomLibrary/Collider.h>

#include <pugixml.hpp>

#include "line.h"
#include "texture.h"
#include "event.h"

using namespace ctl;

struct StrokeContext
{
	std::vector<mth::Point<int>> circle_pattern;

	sdl::Window *  w;
	sdl::Renderer *r;

	const sdl::Camera2D *cam;

	std::vector<Line<float>>	lines;
	std::vector<Texture<float>> textures;

	Line<float>	 target_line;
	Texture<int> target_texture;
};

void erase(StrokeContext *c, size_t i)
{
	assert((c->textures.size() == c->lines.size()) && "Textures and lines not same size.");
	assert(!(c->textures.empty() || c->lines.empty()) && "Textures or lines empty.");

	std::swap(c->textures[i], c->textures.back());
	std::swap(c->lines[i], c->lines.back());

	c->textures.erase(c->textures.end() - 1);
	c->lines.erase(c->lines.end() - 1);
}

auto empty_texture(const StrokeContext *c) -> Texture<int>
{
	int w, h;
	SDL_GetWindowSize(c->w->get(), &w, &h);
	return { .dim = { 0, 0, w, h }, .data = sdl::create_empty(c->r->get(), w, h) };
}

auto shrink_to_fit(const StrokeContext *c) -> Texture<float>
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

	return { .dim = { rect.x, rect.y, rect.w, rect.h }, .data = sdl::crop(c->r->get(), c->target_texture.data, rect) };
}

void push_target(StrokeContext *c)
{
	auto tex = shrink_to_fit(c);

	c->textures.push_back({ .dim = c->cam->screen_world(tex.dim), .data = std::move(tex.data) });

	std::vector<mth::Point<float>> points;
	points.reserve(c->target_line.points.size());
	for (auto p : c->target_line.points) points.emplace_back(c->cam->screen_world(p));

	c->lines.push_back({ c->target_line.radius, c->target_line.color, std::move(points) });
	c->target_line.points.clear();
}

void draw_circle(const StrokeContext *c, const mth::Point<int> mouse)
{
	std::vector<SDL_Point> buf(c->circle_pattern.size());
	std::transform(c->circle_pattern.begin(), c->circle_pattern.end(), buf.begin(), [&mouse](mth::Point<int> p) {
		return SDL_Point{ p.x + mouse.x, p.y + mouse.y };
	});
	SDL_RenderDrawPoints(c->r->get(), buf.data(), c->circle_pattern.size());
}

void draw_connecting_line(const StrokeContext *c, const mth::Point<int> from, const mth::Point<int> to)
{
	for (int i = -c->target_line.radius, end = c->target_line.radius; i < end; ++i)
	{
		SDL_RenderDrawLine(c->r->get(), from.x, from.y + i, to.x, to.y + i);
		SDL_RenderDrawLine(c->r->get(), from.x + i, from.y, to.x + i, to.y);
	}
}

void add_stroke(StrokeContext *c, mth::Point<int> to)
{
	SDL_SetRenderTarget(c->r->get(), c->target_texture.data.get());
	SDL_SetRenderDrawColor(c->r->get(), c->target_line.color.r, c->target_line.color.g, c->target_line.color.b,
						   c->target_line.color.a);

	draw_circle(c, to);
	draw_connecting_line(c, c->target_line.points.back(), to);

	SDL_SetRenderTarget(c->r->get(), nullptr);

	c->target_line.points.emplace_back(to);
}

void start_stroke(StrokeContext *c)
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

auto find_intersections(const StrokeContext *c, const mth::Point<float> p) -> std::vector<size_t>
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

void redraw(StrokeContext *c)
{
	for (size_t i = 0; i < c->textures.size(); ++i)
	{
		auto t = sdl::create_empty(c->r->get(), c->textures[i].dim.w, c->textures[i].dim.h);

		SDL_SetRenderTarget(c->r->get(), t.get());
		SDL_SetRenderDrawColor(c->r->get(), c->lines[i].color.r, c->lines[i].color.g, c->lines[i].color.b,
							   c->lines[i].color.a);

		draw_circle(c, c->lines[i].points.front() - c->textures[i].dim.pos());

		for (auto p = c->lines[i].points.begin() + 1; p != c->lines[i].points.end(); ++i)
		{
			draw_connecting_line(c, *(p - 1) - c->textures[i].dim.pos(), *p - c->textures[i].dim.pos());
			draw_circle(c, *p - c->textures[i].dim.pos());
		}
	}

	SDL_SetRenderTarget(c->r->get(), nullptr);
}

auto regen_pattern(int r) noexcept { return mth::gen_circle_filled(r); }

class Strokes
{
public:
	explicit Strokes(sdl::Window *w, sdl::Renderer *r, const sdl::Camera2D *cam)
	{
		m_con.w = w;
		m_con.r = r;

		m_con.cam = cam;

		m_con.circle_pattern = regen_pattern(m_con.target_line.radius);
	}

	void draw()
	{
		for (const auto &t : m_con.textures)
		{
			const auto world = m_con.cam->world_screen(mth::Rect{ t.dim.x, t.dim.y, t.dim.w, t.dim.h });
			SDL_RenderCopy(m_con.r->get(), t.data.get(), nullptr, &sdl::to_rect(world));
		}

		if (!m_con.target_line.points.empty())
			SDL_RenderCopy(m_con.r->get(), m_con.target_texture.data.get(), nullptr,
						   &sdl::to_rect(m_con.target_texture.dim));
	}

	void event(const SDL_Event &e, const KeyEvent &ke)
	{
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: start_stroke(&m_con); break;
			}

			break;

		case SDL_MOUSEBUTTONUP:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT: push_target(&m_con); break;
			}

			break;

		case SDL_MOUSEMOTION:
			if (ke.test(KeyEventMap::MOUSE_LEFT))
			{
				add_stroke(&m_con, { e.motion.x, e.motion.y });
				trace_point(&m_con.target_line, { e.motion.x, e.motion.y });
			}

			else if (ke.test(KeyEventMap::MOUSE_RIGHT))
				for (size_t i :
					 find_intersections(&m_con, m_con.cam->screen_world(mth::Point{ e.motion.x, e.motion.y })))
					erase(&m_con, i);

			break;

		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
			case SDLK_DOWN:
				m_con.target_line.radius =
					std::clamp(m_con.target_line.radius + (e.key.keysym.sym == SDLK_UP ? 1 : -1), 0, 10);

				std::clog << "Radius: " << +m_con.target_line.radius << std::endl;

				m_con.circle_pattern = regen_pattern(m_con.target_line.radius);

				break;

			case SDLK_r: m_con.target_line.color = sdl::RED; break;
			case SDLK_b: m_con.target_line.color = sdl::BLACK; break;
			}
		}
	}

	void save(pugi::xml_node &node) const
	{
		for (auto [iter_l, iter_t] = std::pair(m_con.lines.begin(), m_con.textures.begin());
			 iter_l != m_con.lines.end(); ++iter_l, ++iter_t)
		{
			auto lines_node = node.append_child("ls");

			lines_node.append_attribute("rad") = iter_l->radius;
			lines_node.append_attribute("col") = *(uint32_t *)&iter_l->color;

			lines_node.append_attribute("x") = iter_t->dim.x;
			lines_node.append_attribute("y") = iter_t->dim.y;
			lines_node.append_attribute("w") = iter_t->dim.w;
			lines_node.append_attribute("h") = iter_t->dim.h;

			for (const auto &l : iter_l->points)
			{
				auto line_node					= lines_node.append_child("l");
				line_node.append_attribute("x") = l.x;
				line_node.append_attribute("y") = l.y;
			}
		}
	}

	void load(pugi::xml_node &node)
	{
		for (auto ls = node.first_child(); ls != nullptr; ls = ls.next_sibling())
		{
			uint8_t radius = ls.attribute("rad").as_uint();

			const auto c	 = ls.attribute("col").as_uint();
			SDL_Color  color = *(const SDL_Color *)&c;

			m_con.textures.push_back({ .dim = { ls.attribute("x").as_float(), ls.attribute("y").as_float(),
												ls.attribute("w").as_float(), ls.attribute("h").as_float() } });

			std::vector<mth::Point<float>> ps;
			for (auto l = ls.first_child(); l != nullptr; l = l.next_sibling())
				ps.emplace_back(l.attribute("x").as_float(), l.attribute("y").as_float());

			m_con.lines.push_back({ radius, color, std::move(ps) });
		}

		redraw(&m_con);
	}

private:
	StrokeContext m_con;
};
