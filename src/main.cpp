#include <SDL.h>
#include <CustomLibrary/SDL/All.h>

#include "app.h"

auto main() -> int
{
	sdl::SDL s;
	App		 a;
	sdl::run(&a, 61);

	return 0;
}