#include <SDL.h>
#include <CustomLibrary/SDL/All.h>

#include "app.h"

auto main(int argc, char **argv) -> int
{
	try
	{
		sdl::SDL s;
		App		 a;
		sdl::run(&a, 61);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}

	return 0;
}

