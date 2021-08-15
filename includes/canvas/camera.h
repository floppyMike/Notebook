#pragma once

#include "window.h"

#include "layout.h"

void zoom(const Window &w, CanvasContext &c, int factor, float scale)
{
	const auto mouse_p = w.get_mousepos();
	c.cam.zoom(1.F + factor / scale, mouse_p);
}
