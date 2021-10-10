#pragma once

#include <vector>

#include <CustomLibrary/SDL/All.h>

#include "renderer.h"
#include "status.h"

using namespace ctl;

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------

struct WorldTexture
{
	mth::Rect<float>  dim;
	Renderer::Texture data;
};

struct ScreenTexture
{
	mth::Rect<int>		   dim;
	Renderer::CacheTexture data;
};

// -----------------------------------------------------------------------------
// Lines
// -----------------------------------------------------------------------------

struct WorldLine
{
	std::vector<mth::Point<float>> points;
	size_t						   idx;
};

struct WorldLineInfo
{
	float	  radius; // The radius is screen == world
	float	  scale;
	SDL_Color color;
};

struct ScreenLine
{
	std::vector<mth::Point<int>> points;
};

struct ScreenLineInfo
{
	SDL_Color color = sdl::BLACK;

	int	  i_rad;
	float radius;
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
	Renderer::Font data;
};

// -----------------------------------------------------------------------------
// Selection
// -----------------------------------------------------------------------------

enum class CanvasType
{
	STROKE,
	TEXT,
	NONE,
};

struct Select
{
	size_t		  idx  = -1;
	WorldTexture *wt   = nullptr;
	CanvasType	  type = CanvasType::NONE;
};

// -----------------------------------------------------------------------------
// Debug
// -----------------------------------------------------------------------------

struct CanvasDebug
{
	Renderer::Font	  font;
	Renderer::Texture mouse;
};

// -----------------------------------------------------------------------------
// DBs
// -----------------------------------------------------------------------------

using WorldTextureDB = std::vector<WorldTexture>;

using WorldLineDB	  = std::vector<WorldLine>;
using WorldLineInfoDB = std::vector<WorldLineInfo>;

using WorldTextInfoDB = std::vector<WorldTextInfo>;

using IndexDB = std::vector<size_t>;

// -----------------------------------------------------------------------------
// Context
// -----------------------------------------------------------------------------

struct SaveState
{
	CanvasStatus status = CanvasStatus::PAINTING;

	sdl::Camera2D cam;

	WorldLineDB		swls;
	WorldLineInfoDB swlis;
	// WorldTextureDB	swts;

	WorldTextureDB	wts;

	WorldTextureDB	txwts;
	WorldTextInfoDB txwtxis;
};

struct CanvasContext : SaveState
{
	ScreenLine	   ssl;
	ScreenLineInfo ssli;
	ScreenTexture  sst;

	TextFont txf;

	Select select;

	std::optional<mth::Point<float>> start_mp;

	std::string save_path;

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
