#pragma once

#include "line.h"
#include "texture.h"

struct Canvas
{
	sdl::Camera2D cam;

	std::vector<Line<float>>	lines;
	std::vector<Texture<float>> textures;

	void emplace(Texture<int> &&t, Line<float> &&l)
	{
		textures.push_back({ .dim = cam.screen_world(t.dim), .data = std::move(t.data) });

		std::vector<mth::Point<float>> points;
		points.reserve(l.points.size());
		for (auto p : l.points) points.emplace_back(cam.screen_world(p));

		lines.push_back({ l.radius, l.color, std::move(points) });
	}

	void erase(size_t i)
	{
		std::swap(textures[i], textures.back());
		std::swap(lines[i], lines.back());
		textures.erase(textures.end() - 1);
		lines.erase(lines.end() - 1);
	}

	void zoom(int factor, float scale)
	{
		mth::Point<int> mouse_p;
		SDL_GetMouseState(&mouse_p.x, &mouse_p.y);
		cam.zoom(1.F + factor / scale, mouse_p);
	}

	friend auto operator<<(sdl::Renderer &r, const Canvas &c) -> sdl::Renderer &
	{
		for (const auto &t : c.textures)
		{
			const auto world = c.cam.world_screen(mth::Rect{ t.dim.x, t.dim.y, t.dim.w, t.dim.h });
			SDL_RenderCopy(r.get(), t.data.get(), nullptr, &sdl::to_rect(world));
		}

		return r;
	}
};
