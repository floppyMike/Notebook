#pragma once

#include <CustomLibrary/SDL/All.h>

#include "comp/renderer.h"
#include "line.h"

using namespace ctl;

template<typename T>
struct Texture
{
	mth::Rect<T, T> dim;
	TextureData	data;
};

