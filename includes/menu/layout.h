#pragma once

#include "renderer.h"
#include "status.h"

#include <CustomLibrary/Geometry.h>

using namespace ctl;

static constexpr auto MAX_ICON	 = 10;
static constexpr auto ICON_SIZE	 = 32;
static constexpr auto SEPERATION = 10;

struct Icon
{
	mth::Rect<int> dim;
};

struct Bar
{
	mth::Rect<int>	  dim;
	Renderer::Texture data;
};

using IconDB = std::vector<Icon>;

struct BarContext
{
	Renderer::Texture icon_map;

	Bar	   bar;
	IconDB icons = IconDB(EVENT_ALL); // Preload all icons
};
