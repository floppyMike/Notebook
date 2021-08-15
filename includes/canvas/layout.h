#pragma once

#include <vector>

#include <CustomLibrary/SDL/All.h>

#include "line.h"
#include "texture.h"

using namespace ctl;

struct CanvasContext
{
	sdl::Camera2D cam;

	std::vector<Line<float>>	lines;
	std::vector<Texture<float>> textures;

	Line<float>	 target_line;
	Texture<int> target_texture;
};
