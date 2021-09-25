#include <SDL.h>
#include <SDL_ttf.h>
#include <CustomLibrary/SDL/All.h>

#include "CustomLibrary/IO.h"
#include "app.h"


auto main(int argc, char **argv) -> int
{
	try
	{
		sdl::SDL	 s;
		sdl::SDL_TTF ttf;

		App a;
		sdl::run(&a, 61);
	}
	catch (const std::exception &e)
	{
		ctl::print("Unresolvable error: %s\n", e.what());
	}

	return 0;
}
