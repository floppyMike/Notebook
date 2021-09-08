#pragma once

#include <vector>

#include <CustomLibrary/SDL/All.h>

#include "renderer.h"
#include "status.h"

using namespace ctl;

// -----------------------------------------------------------------------------
// Lines
// -----------------------------------------------------------------------------

struct WorldLine
{
	std::vector<mth::Point<float>> points;
};

struct WorldLineInfo
{
	float	  radius;
	float	  scale;
	SDL_Color color;
};

struct ScreenLine
{
	std::vector<mth::Point<int>> points;
};

struct ScreenLineInfo
{
	uint8_t	  radius = 3;
	SDL_Color color	 = sdl::BLACK;
};

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------

struct WorldTexture
{
	mth::Rect<float> dim;
	TextureData		 data;
};

struct ScreenTexture
{
	mth::Rect<int> dim;
	TextureData	   data;
};

// -----------------------------------------------------------------------------
// Text
// -----------------------------------------------------------------------------

struct WorldTextInfo
{
	std::string str;
	float		scale;
};

struct TextFont
{
	FontData data;
};

// -----------------------------------------------------------------------------
// Selection
// -----------------------------------------------------------------------------

enum CanvasType
{
	STROKE,
	TEXT,
};

struct Select
{
	WorldTexture *wts;
	CanvasType	  type;
};

// -----------------------------------------------------------------------------
// DBs
// -----------------------------------------------------------------------------

using WorldTextureDB = std::vector<WorldTexture>;

using WorldLineDB	  = std::vector<WorldLine>;
using WorldLineInfoDB = std::vector<WorldLineInfo>;

using WorldTextInfoDB = std::vector<WorldTextInfo>;

// -----------------------------------------------------------------------------
// Context
// -----------------------------------------------------------------------------

struct CanvasContext
{
	CanvasStatus status = PAINTING;

	sdl::Camera2D cam;

	WorldLineDB		swls;
	WorldLineInfoDB swlis;
	WorldTextureDB	swts;

	ScreenLine	   ssl;
	ScreenLineInfo ssli;
	ScreenTexture  sst;

	size_t txe;
	WorldTextureDB	txwts;
	WorldTextInfoDB txwtxis;

	TextFont txf;

	Select select;
};
