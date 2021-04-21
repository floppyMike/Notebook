#pragma once

#include <CustomLibrary/SDL/All.h>

using namespace ctl;

template<typename T>
struct Line
{
	uint8_t					   radius = 1;
	SDL_Color				   color  = sdl::BLACK;
	std::vector<mth::Point<T>> points;
};
