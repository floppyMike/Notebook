#pragma once

#include <CustomLibrary/SDL/All.h>

#include "comp/window.h"
#include "comp/renderer.h"
#include "event.h"

struct EventPipe
{
	const SDL_Event &e;
	const KeyEvent &ke;
	const Window *const w;
	const Renderer *const r;
};
