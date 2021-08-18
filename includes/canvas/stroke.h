#pragma once

#include <algorithm>

#include <CustomLibrary/Collider.h>

#include "window.h"
#include "line.h"
#include "texture.h"
#include "event.h"
#include "layout.h"

using namespace ctl;

/**
 * @brief Shrink texture to draw area
 *
 * @param r Generate new texture
 * @param c	Access lines array
 *
 * @return Shrunk texture
 */
auto shrink_to_fit(const Renderer &r, const CanvasContext &c) -> Texture<int>
{
	SDL_Point min = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
	SDL_Point max = { 0, 0 };

	for (const auto &p : c.target_line.points) // Find line range inside texture
	{
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
	}

	min.x -= c.target_line.radius;
	min.y -= c.target_line.radius;
	max.x += c.target_line.radius;
	max.y += c.target_line.radius;

	const auto w = max.x - min.x;
	const auto h = max.y - min.y;

	const mth::Rect rect = { static_cast<int>(min.x), static_cast<int>(min.y), static_cast<int>(w),
							 static_cast<int>(h) };

	return { .dim = { rect.x, rect.y, rect.w, rect.h }, .data = r.crop_texture(c.target_texture.data, rect) };
}

/**
 * @brief Quick erase texture at index
 *
 * @param c Using texture and lines array
 * @param i index to delete
 */
void erase(CanvasContext &c, size_t i)
{
	assert((c.textures.size() == c.lines.size()) && "Textures and lines not same size.");
	assert(!(c.textures.empty() || c.lines.empty()) && "Textures or lines empty.");

	std::swap(c.textures[i], c.textures.back());
	std::swap(c.lines[i], c.lines.back());

	c.textures.erase(c.textures.end() - 1);
	c.lines.erase(c.lines.end() - 1);
}

/**
 * @brief Allocate a new target
 *
 * @param w Get the window size
 * @param r Allocate a new texture
 * @param c Create new target
 */
void initialize_stroke(const Window &w, const Renderer &r, CanvasContext &c)
{
	const auto w_size = w.get_windowsize();
	c.target_texture  = { .dim = { 0, 0, w_size.w, w_size.h }, .data = r.create_texture(w_size.w, w_size.h) };
}

void render_start_stroke(const Window &w, Renderer &r, CanvasContext &c, mth::Point<int> p)
{
	r.set_draw_color(c.target_line.color);

	r.set_target();
	r.render_conn_stroke(p, c.target_line.radius);
	r.set_target(c.target_texture.data);
	r.render_conn_stroke(p, c.target_line.radius);
}

/**
 * @brief Start drawing
 *
 * @param w Get relative mouse position
 * @param r Render to texture and window
 * @param c Fill out target data
 */
void start_stroke(const Window &w, Renderer &r, CanvasContext &c)
{
	const auto mp = w.get_mousepos();

	c.target_line.points.emplace_back(mp.x, mp.y);
	render_start_stroke(w, r, c, mp);
}

/**
 * @brief Add & draw new location to target
 *
 * @param w Get mouse position
 * @param r Render line to texture and window
 * @param c Add target data
 */
void connect_stroke(const Window &w, Renderer &r, CanvasContext &c)
{
	const auto mp = w.get_mousepos();

	r.set_draw_color(c.target_line.color);

	r.set_target();
	r.render_inter_stroke(c.target_line.points.back(), mp, c.target_line.radius);
	r.set_target(c.target_texture.data);
	r.render_inter_stroke(c.target_line.points.back(), mp, c.target_line.radius);

	c.target_line.points.emplace_back(mp);
}

/**
 * @brief Finalize target and store it
 *
 * @param r Texture manipulation
 * @param cam Calculating world
 * @param c Storing texture and line
 */
void finalize_stroke(const Window &w, Renderer &r, CanvasContext &c)
{
	render_start_stroke(w, r, c, w.get_mousepos());
	
	auto tex = shrink_to_fit(r, c);

	c.textures.push_back({ .dim = c.cam.screen_world(tex.dim), .data = std::move(tex.data) });

	std::vector<mth::Point<float>> points;
	points.reserve(c.target_line.points.size());
	for (auto p : c.target_line.points) points.emplace_back(c.cam.screen_world(p));

	c.lines.push_back({ c.target_line.radius, c.target_line.color, std::move(points) });

	c.target_line.points.clear();
	c.target_texture.data.reset();
}

auto find_intersections(const CanvasContext &c, const mth::Point<float> p) -> std::vector<size_t>
{
	std::vector<size_t> res;

	for (int i = c.textures.size() - 1; i >= 0; --i)
		if (mth::collision(p, c.textures[i].dim))
			res.emplace_back(i);

	res.erase(
		std::remove_if(res.begin(), res.end(),
					   [&c, p](const auto i)
					   {
						   return std::none_of(
							   c.lines[i].points.begin(), c.lines[i].points.end(),
							   [&c, i, p](const mth::Point<float> &l)
							   {
								   const mth::Rect box = { l.x, l.y, c.lines[i].radius * 2, c.lines[i].radius * 2 };
								   return mth::collision(p, box);
							   });
					   }),
		res.end());

	return res;
}

void handle_stroke_event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_MOUSEBUTTONDOWN:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			initialize_stroke(w, r, c);
			start_stroke(w, r, c);
			break;
		}

		break;

	case SDL_MOUSEBUTTONUP:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT: finalize_stroke(w, r, c); break;
		}

		break;

	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_LEFT))
		{
			connect_stroke(w, r, c);
		}

		// 		else if (ke.test(KeyEventMap::MOUSE_RIGHT))
		// 			for (size_t i : find_intersections(&m_con, m_con.cam->screen_world(mth::Point{ e.motion.x,
		// e.motion.y
		// }))) 				erase(&m_con, i);

		break;

	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
		case SDLK_UP:
		case SDLK_DOWN:
			c.target_line.radius = std::clamp(c.target_line.radius + (e.key.keysym.sym == SDLK_UP ? 1 : -1), 0, 10);

			std::clog << "Radius: " << +c.target_line.radius << std::endl;

			r.set_stroke_radius(c.target_line.radius);

			break;

		case SDLK_r: c.target_line.color = sdl::RED; break;
		case SDLK_b: c.target_line.color = sdl::BLACK; break;
		}
	}
}

// void redraw(StrokeContext *c)
// {
// 	for (size_t i = 0; i < c->textures.size(); ++i)
// 	{
// 		auto t = sdl::create_empty(c->r->get(), c->textures[i].dim.w, c->textures[i].dim.h);
//
// 		SDL_SetRenderTarget(c->r->get(), t.get());
// 		SDL_SetRenderDrawColor(c->r->get(), c->lines[i].color.r, c->lines[i].color.g, c->lines[i].color.b,
// 							   c->lines[i].color.a);
//
// 		draw_circle(c, c->lines[i].points.front() - c->textures[i].dim.pos());
//
// 		for (auto p = c->lines[i].points.begin() + 1; p != c->lines[i].points.end(); ++i)
// 		{
// 			draw_connecting_line(c, *(p - 1) - c->textures[i].dim.pos(), *p - c->textures[i].dim.pos());
// 			draw_circle(c, *p - c->textures[i].dim.pos());
// 		}
// 	}
//
// 	SDL_SetRenderTarget(c->r->get(), nullptr);
// }

// auto regen_pattern(int r) noexcept
// {
// 	return mth::gen_circle_filled(r);
// }

// class Strokes
// {
// public:
// 	Strokes() = default;
//
// 	void draw(const Renderer &r)
// 	{
// 		// 		for (const auto &t : m_con.textures)
// 		// 		{
// 		// 			const auto world = m_con.cam->world_screen(mth::Rect{ t.dim.x, t.dim.y, t.dim.w, t.dim.h });
// 		// 			SDL_RenderCopy(m_con.r->get(), t.data.get(), nullptr, &sdl::to_rect(world));
// 		// 		}
// 		//
// 		// 		if (!m_con.target_line.points.empty())
// 		// 			SDL_RenderCopy(m_con.r->get(), m_con.target_texture.data.get(), nullptr,
// 		// 						   &sdl::to_rect(m_con.target_texture.dim));
// 	}
//
// 	void event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r, const sdl::Camera2D &cam)
// 	{
// 		switch (e.type)
// 		{
// 		case SDL_MOUSEBUTTONDOWN:
// 			switch (e.button.button)
// 			{
// 			case SDL_BUTTON_LEFT: start_stroke(w, r, m_con); break;
// 			}
//
// 			break;
//
// 			// 		case SDL_MOUSEBUTTONUP:
// 			// 			switch (e.button.button)
// 			// 			{
// 			// 			case SDL_BUTTON_LEFT: push_target(&m_con); break;
// 			// 			}
// 			//
// 			// 			break;
// 			//
// 		case SDL_MOUSEMOTION:
// 			if (ke.test(KeyEventMap::MOUSE_LEFT))
// 			{
// 				// 				add_stroke(&m_con, { e.motion.x, e.motion.y });
// 				// 				trace_point(&m_con.target_line, { e.motion.x, e.motion.y });
//
// 				r.set_draw_color(sdl::BLACK);
// 				r.draw_rect(cam.world_screen(mth::Rect{ e.motion.x, e.motion.y, 10, 10 }));
// 				// r.refresh();
// 			}
//
// 			// 			else if (ke.test(KeyEventMap::MOUSE_RIGHT))
// 			// 				for (size_t i :
// 			// 					 find_intersections(&m_con, m_con.cam->screen_world(mth::Point{ e.motion.x, e.motion.y
// 			// }))) 					erase(&m_con, i);
//
// 			break;
// 			//
// 			// 		case SDL_KEYDOWN:
// 			// 			switch (e.key.keysym.sym)
// 			// 			{
// 			// 			case SDLK_UP:
// 			// 			case SDLK_DOWN:
// 			// 				m_con.target_line.radius =
// 			// 					std::clamp(m_con.target_line.radius + (e.key.keysym.sym == SDLK_UP ? 1 : -1), 0, 10);
// 			//
// 			// 				std::clog << "Radius: " << +m_con.target_line.radius << std::endl;
// 			//
// 			// 				m_con.circle_pattern = regen_pattern(m_con.target_line.radius);
// 			//
// 			// 				break;
// 			//
// 			// 			case SDLK_r: m_con.target_line.color = sdl::RED; break;
// 			// 			case SDLK_b: m_con.target_line.color = sdl::BLACK; break;
// 			// 			}
// 		}
// 	}
//
// // 	void save(pugi::xml_node &node) const
// // 	{
// // 		for (auto [iter_l, iter_t] = std::pair(m_con.lines.begin(), m_con.textures.begin());
// // 			 iter_l != m_con.lines.end(); ++iter_l, ++iter_t)
// // 		{
// // 			auto lines_node = node.append_child("ls");
// //
// // 			lines_node.append_attribute("rad") = iter_l->radius;
// // 			lines_node.append_attribute("col") = *(uint32_t *)&iter_l->color;
// //
// // 			lines_node.append_attribute("x") = iter_t->dim.x;
// // 			lines_node.append_attribute("y") = iter_t->dim.y;
// // 			lines_node.append_attribute("w") = iter_t->dim.w;
// // 			lines_node.append_attribute("h") = iter_t->dim.h;
// //
// // 			for (const auto &l : iter_l->points)
// // 			{
// // 				auto line_node					= lines_node.append_child("l");
// // 				line_node.append_attribute("x") = l.x;
// // 				line_node.append_attribute("y") = l.y;
// // 			}
// // 		}
// // 	}
// //
// // 	void load(pugi::xml_node &node)
// // 	{
// // 		for (auto ls = node.first_child(); ls != nullptr; ls = ls.next_sibling())
// // 		{
// // 			uint8_t radius = ls.attribute("rad").as_uint();
// //
// // 			const auto c	 = ls.attribute("col").as_uint();
// // 			SDL_Color  color = *(const SDL_Color *)&c;
// //
// // 			m_con.textures.push_back({ .dim = { ls.attribute("x").as_float(), ls.attribute("y").as_float(),
// // 												ls.attribute("w").as_float(), ls.attribute("h").as_float() } });
// //
// // 			std::vector<mth::Point<float>> ps;
// // 			for (auto l = ls.first_child(); l != nullptr; l = l.next_sibling())
// // 				ps.emplace_back(l.attribute("x").as_float(), l.attribute("y").as_float());
// //
// // 			m_con.lines.push_back({ radius, color, std::move(ps) });
// // 		}
//
// 		// redraw(&m_con);
// 	// }
//
// private:
// 	StrokeContext m_con;
// };
