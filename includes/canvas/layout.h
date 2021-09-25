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

enum class CanvasType
{
	STROKE,
	TEXT,
};

struct Select
{
	WorldTexture *wts = nullptr;
	CanvasType	  type;
};

// -----------------------------------------------------------------------------
// Debug
// -----------------------------------------------------------------------------

struct CanvasDebug
{
	FontData	font;
	TextureData mouse;
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

struct ScreenStroke
{
	ScreenLine	   ssl;
	ScreenLineInfo ssli;
	ScreenTexture  sst;
};

struct EditText
{
	WorldTexture  txet;
	WorldTextInfo txei;

	TextFont txf;
};

struct SaveState
{
	CanvasStatus status = CanvasStatus::PAINTING;

	sdl::Camera2D cam;

	WorldLineDB		swls;
	WorldLineInfoDB swlis;
	WorldTextureDB	swts;

	WorldTextureDB	txwts;
	WorldTextInfoDB txwtxis;
};

struct CanvasContext
	: ScreenStroke
	, EditText
	, SaveState
{
	Select select;

#ifndef NDEBUG
	CanvasDebug debug;
#endif
};

// -----------------------------------------------------------------------------
// Util
// -----------------------------------------------------------------------------

/**
 * @brief Quick erase array at index
 *
 * @param i index to delete
 * @param arr target vector
 * @param arrs other vectors to target
 */
template<typename U, typename... T>
inline void erase(size_t i, U &arr, T &...arrs)
{
	assert(((arr.size() == arrs.size()) && ...) && "Arrays not same length.");
	assert(!(arr.empty() || (arrs.empty() || ...)) && "One of arrays empty.");

	std::swap(arr[i], arr.back());
	(std::swap(arrs[i], arrs.back()), ...);

	arr.erase(arr.end() - 1);
	(arrs.erase(arrs.end() - 1), ...);
}

/**
 * @brief Clear vectors
 *
 * @param arr array to clear
 * @param arrs other arrays to clear
 */
template<typename U, typename... T>
inline void clear(U &arr, T &...arrs)
{
	arr.clear();
	(arrs.clear(), ...);
}
