#pragma once

#include <CustomLibrary/SDL/All.h>
#include <CustomLibrary/Error.h>

#include "line.h"

using namespace ctl;

template<typename T>
struct Texture
{
	mth::Rect<T, T> dim;
	sdl::Texture	data;
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

auto shrink_to_fit(SDL_Renderer *r, const Line<int> &l, const sdl::Texture &t) -> Texture<int>
{
	SDL_Point min = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() }, max = { 0, 0 };

	for (const auto &p : l.points)
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

	sdl::Texture   tex	= create_empty(r, w, h);
	const SDL_Rect rect = { min.x, min.y, w, h };

	SDL_SetRenderTarget(r, tex.get());
	SDL_SetRenderDrawColor(r, sdl::BLACK.r, sdl::BLACK.g, sdl::BLACK.b, sdl::BLACK.a);
	SDL_RenderCopy(r, t.get(), &rect, nullptr);
	SDL_SetRenderTarget(r, nullptr);

	return { .dim = { rect.x, rect.y, rect.w, rect.h }, .data = std::move(tex) };
}
