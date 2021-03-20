#include <SDL.h>
#include <CustomLibrary/SDL/All.h>

#include <CustomLibrary/Error.h>

using namespace ctl;

struct Line
{
	uint8_t				   radius = 1;
	std::vector<SDL_Point> points;
};

struct Texture
{
	SDL_Rect	 dim;
	sdl::Texture data;
};

auto create_empty(SDL_Renderer *r, int w, int h) noexcept -> sdl::Texture
{
	sdl::Texture t;
	ASSERT(t = sdl::Texture(SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h)),
		   SDL_GetError());

	SDL_SetTextureBlendMode(t.get(), SDL_BLENDMODE_BLEND);

	SDL_SetRenderTarget(r, t.get());
	SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
	SDL_RenderClear(r);
	SDL_SetRenderTarget(r, nullptr);

	return t;
}

auto shrink_to_fit(SDL_Renderer *r, const Line &l, const sdl::Texture &t) -> Texture
{
	SDL_Point min = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() }, max = { 0, 0 };
	for (const SDL_Point &p : l.points)
	{
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
	}

	min.x -= l.radius;
	min.y -= l.radius;
	max.x += l.radius;
	max.y += l.radius;

	const auto w = max.x - min.x + l.radius * 2, h = max.y - min.y + l.radius * 2;
	Texture	   tex = { .dim = { .x = min.x, .y = min.y, .w = w, .h = h }, .data = create_empty(r, w, h) };

	SDL_SetRenderTarget(r, tex.data.get());
	SDL_SetRenderDrawColor(r, sdl::BLACK.r, sdl::BLACK.g, sdl::BLACK.b, sdl::BLACK.a);
	SDL_RenderCopy(r, t.get(), &tex.dim, nullptr);
	SDL_SetRenderTarget(r, nullptr);

	return tex;
}

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
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				m_press_left	 = true;
				m_target_texture = create_empty(m_rend.get(), 640, 480);
			}
			else if (e.button.button == SDL_BUTTON_RIGHT)
			{
				m_press_right = true;
			}

			break;

		case SDL_MOUSEBUTTONUP:
			if (m_press_left)
			{
				m_press_left = false;
				m_textures.push_back(shrink_to_fit(m_rend.get(), m_target_line, m_target_texture));
				m_lines.push_back(std::move(m_target_line));
			}
			else if (m_press_right)
			{
				m_press_right = false;
			}

			break;

		case SDL_MOUSEMOTION:
			if (m_press_left)
			{
				SDL_SetRenderTarget(m_rend.get(), m_target_texture.get());

				SDL_SetRenderDrawColor(m_rend.get(), sdl::BLACK.r, sdl::BLACK.g, sdl::BLACK.b, sdl::BLACK.a);

				draw_circle({ e.motion.x, e.motion.y });
				draw_connecting_line({ e.motion.x - e.motion.xrel, e.motion.y - e.motion.yrel },
									 { e.motion.x, e.motion.y });

				m_target_line.points.push_back(
					{ e.motion.x - m_target_line.radius, e.motion.y - m_target_line.radius });

				SDL_SetRenderTarget(m_rend.get(), nullptr);
			}
			if (m_press_right)
			{
				const auto intersects = find_intersections({ e.motion.x, e.motion.y });

				for (size_t i : intersects)
				{
					std::swap(m_textures[i], m_textures.back());
					std::swap(m_lines[i], m_lines.back());
					m_textures.erase(m_textures.end() - 1);
					m_lines.erase(m_lines.end() - 1);
				}
			}

			break;

		case SDL_MOUSEWHEEL:
			m_target_line.radius += e.wheel.y;
			m_circle_pattern = generate_draw_circle(m_target_line.radius);
			break;
		}
	}
	void update() {}
	void render()
	{
		SDL_SetRenderDrawColor(m_rend.get(), sdl::WHITE.r, sdl::WHITE.g, sdl::WHITE.b, sdl::WHITE.a);

		SDL_RenderClear(m_rend.get());

		for (const Texture &t : m_textures) SDL_RenderCopy(m_rend.get(), t.data.get(), nullptr, &t.dim);

		if (!m_target_line.points.empty())
			SDL_RenderCopy(m_rend.get(), m_target_texture.get(), nullptr, nullptr);

		SDL_RenderPresent(m_rend.get());
	}

private:
	sdl::Window	  m_win;
	sdl::Renderer m_rend;

	sdl::Texture m_target_texture;
	Line		 m_target_line;

	bool m_press_left  = false;
	bool m_press_right = false;

	std::vector<Texture> m_textures;
	std::vector<Line>	 m_lines;

	std::vector<mth::Point<int>> m_circle_pattern;

	void draw_circle(const SDL_Point mouse) const
	{
		std::vector<SDL_Point> buf(m_circle_pattern.size());
		std::transform(m_circle_pattern.begin(), m_circle_pattern.end(), buf.begin(), [&mouse](mth::Point<int> p) {
			return SDL_Point{ p.x + mouse.x, p.y + mouse.y };
		});
		SDL_RenderDrawPoints(m_rend.get(), buf.data(), m_circle_pattern.size());
	}

	void draw_connecting_line(const SDL_Point from, const SDL_Point to) const
	{
		// Draw connecting line
		for (int i = -m_target_line.radius, end = m_target_line.radius; i < end; ++i)
		{
			SDL_RenderDrawLine(m_rend.get(), from.x, from.y + i, to.x, to.y + i);
			SDL_RenderDrawLine(m_rend.get(), from.x + i, from.y, to.x + i, to.y);
		}
	}

	[[nodiscard]] auto find_intersections(const SDL_Point p) const -> std::vector<size_t>
	{
		std::vector<size_t> res;

		for (size_t i = 0; i < m_textures.size(); ++i)
			if (SDL_PointInRect(&p, &m_textures[i].dim))
				res.emplace_back(i);

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

		return res;
	}
};

auto main() -> int
{
	sdl::SDL s;
	App		 a;
	sdl::run(&a, 61);

	return 0;
}
