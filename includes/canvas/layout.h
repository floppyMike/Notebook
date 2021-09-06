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
	uint8_t	  radius = 1;
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

struct ScreenTextInfo
{
	std::string str;
};

struct TextFont
{
	FontData data;
};

// -----------------------------------------------------------------------------
// DBs
// -----------------------------------------------------------------------------

using WorldLineDB	  = std::vector<WorldLine>;
using WorldLineInfoDB = std::vector<WorldLineInfo>;
using WorldTextureDB  = std::vector<WorldTexture>;

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

	std::vector<WorldTexture>  texts;
	std::vector<WorldTextInfo> text_strs;

	TextFont	   target_font;
	ScreenTextInfo target_text_str;
	ScreenTexture  target_text;

	ScreenTexture select_texture;
	ScreenLine	  select_line;
};
