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
