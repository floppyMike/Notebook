#pragma once

#include "renderer/sdl2.h"
#include <CustomLibrary/Geometry.h>
#include <CustomLibrary/SDL/Event.h>

using namespace ctl;

static constexpr auto MAX_ICON	 = 10;
static constexpr auto ICON_SIZE	 = 32;
static constexpr auto SEPERATION = 10;

enum MenuID
{
	ID_DRAW,
	ID_SELECT,
	ID_TEXT,

	ID_ALL,
};

struct Icon
{
	mth::Rect<int> dim;
	int			   id;
};

struct Bar
{
	mth::Rect<int> dim;
	TextureData	   data;
};

using IconDB = std::vector<Icon>;

struct BarContext
{
	TextureData icon_map;
	Bar			bar;
	IconDB		icons;

	const Icon *hover = nullptr;
};
