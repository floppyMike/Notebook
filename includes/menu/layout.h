#pragma once

#include "renderer/sdl2.h"
#include <CustomLibrary/Geometry.h>

using namespace ctl;

static constexpr auto MAX_ICON	 = 10;
static constexpr auto ICON_SIZE	 = 32;
static constexpr auto SEPERATION = 10;

enum MenuIDs
{
	ID_DRAW,
	ID_SELECT,
	ID_TEXT,
};

struct Icon
{
	mth::Rect<int> dim;
	int			   id;
};

struct BarContext
{
	TextureData icon_map;

	mth::Point<int>	  bar_loc;
	std::vector<Icon> icons;
};
