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
	uint8_t	  radius = 1;
	float	  scale;
	SDL_Color color = sdl::BLACK;
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
	mth::Rect<float, float> dim;
	TextureData				data;
};

struct ScreenTexture
{
	mth::Rect<int, int> dim;
	TextureData			data;
};

// -----------------------------------------------------------------------------
// Text
// -----------------------------------------------------------------------------

struct TextInfo
{
	std::string str;
	float scale;
};

struct TextFont
{
	FontData data;
};

// -----------------------------------------------------------------------------
// Context
// -----------------------------------------------------------------------------

struct CanvasContext
{
	Status status = PAINTING;

	sdl::Camera2D cam;

	std::vector<WorldLine>	   lines;
	std::vector<WorldLineInfo> lines_info;
	std::vector<WorldTexture>  textures;

	ScreenLine	   target_line;
	ScreenLineInfo target_line_info;
	ScreenTexture  target_texture;

	TextFont font;

	std::vector<WorldTexture> texts;
	std::vector<TextInfo>	  text_strs;
};
