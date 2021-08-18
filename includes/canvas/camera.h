#pragma once

#include "window.h"
#include "event.h"
#include "layout.h"

/**
 * @brief Handle camera motion
 *
 * @param e Listen for mouse event
 * @param ke If key pressed on motion
 * @param w Mouse position relative to window
 * @param r Signal redraw
 * @param c Modify camera
 */
void handle_cam_event(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_MIDDLE))
		{
			c.cam.translate(-e.motion.xrel, -e.motion.yrel);
			r.refresh();
		}

		break;

	case SDL_MOUSEWHEEL:
		c.cam.zoom(1.F + e.wheel.y / 10.F, w.get_mousepos());
		r.refresh();

		break;
	}
}
