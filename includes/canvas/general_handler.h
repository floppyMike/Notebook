#pragma once

#include "event.h"
#include "status.h"
#include "window.h"
#include "renderer.h"

#include "layout.h"
#include "stroke.h"
#include "save.h"
#include "text.h"

/**
 * @brief Zoom the camera onto the mouse point
 */
inline void zoom_camera(CanvasContext &c, float strength)
{
	const auto s = std::clamp(c.cam.scale * (1.F + strength / 10.F), 0.1F, 10.F);
	c.cam.set_zoom(s, sdl::mouse_position());
	change_radius(c.cam, c.ssli, c.ssli.i_rad);
}

/**
 * @brief Move the camera using delta
 */
inline void move_camera(CanvasContext &c, float dx, float dy)
{
	c.cam.translate(dx, dy);
}

/**
 * @brief Check if the quicksave filename if cached
 */
inline auto got_filename(CanvasContext &c)
{
	return !c.save_path.empty();
}

/**
 * @brief Reload the textures based on their info
 */
inline void recreate_textures(Renderer &r, CanvasContext &c)
{
	regen_strokes(r, c.swts, c.swls, c.swlis);
	regen_texts(r, c.txf, c.txwts, c.txwtxis);
}

/**
 * @brief Store the temporary filename
 */
inline void cache_filename(CanvasContext &c, const char *f)
{
	c.save_path = f;
}

/**
 * @brief Handle window events for camera & saving/loading
 */
inline void handle_general(const SDL_Event &e, const KeyEvent &ke, const Window &w, Renderer &r, CanvasContext &c)
{
	switch (e.type)
	{
	case SDL_MOUSEMOTION:
		if (ke.test(KeyEventMap::MOUSE_MIDDLE))
		{
			move_camera(c, (float)-e.motion.xrel, (float)-e.motion.yrel);
			r.refresh();
		}

		break;

	case SDL_MOUSEWHEEL:
	{
		zoom_camera(c, (float)e.wheel.y);
		r.refresh();

		break;
	}

	case EVENT_QUICKSAVE:
		if (!got_filename(c))
		{
			sdl::push_event(e.user.windowID, EVENT_SAVE);
			break;
		}

		save(c, c.save_path.c_str());

		break;

	case EVENT_SAVE:
		if (const auto filename = open_file_save(); filename)
		{
			save(c, filename->c_str());
			cache_filename(c, filename->c_str());
		}

		break;

	case EVENT_LOAD:
		if (const auto filename = open_file_load(); filename)
		{
			clear(c.swts, c.swls, c.swlis, c.txwts, c.txwtxis);

			CATCH_LOG(load(c, filename->c_str()));
			recreate_textures(r, c);
			r.refresh();

			cache_filename(c, filename->c_str());
		}

		break;
	}
}
