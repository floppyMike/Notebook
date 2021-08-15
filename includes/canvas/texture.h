#pragma once

#include <CustomLibrary/SDL/All.h>

#include "window.h"
#include "renderer.h"

using namespace ctl;

template<typename T>
struct Texture
{
	mth::Rect<T, T> dim;
	TextureData	data;
};

/**
 * @brief Creates a empty texture with screen size
 *
 * @param w Window ptr for screen size
 * @param r Renderer for generating texture
 *
 * @return Empty texture
 */
auto empty_texture(const Window &w, const Renderer &r) -> Texture<int>
{
	const auto w_size = w.get_windowsize();
	return { .dim = { 0, 0, w_size.w, w_size.h }, .data = r.create_texture(w_size.w, w_size.h) };
}

/**
 * @brief Shrink texture to draw area
 *
 * @param r Generate new texture
 * @param c	Access lines array
 *
 * @return Shrunk texture
 */
auto shrink_to_fit(const Renderer *r, const StrokeContext *c) -> Texture<float>
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

	return { .dim = { rect.x, rect.y, rect.w, rect.h }, .data = r->crop_texture(c->target_texture.data, rect) };
}
