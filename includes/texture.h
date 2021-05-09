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

