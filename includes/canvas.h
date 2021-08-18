#pragma once

#include <CustomLibrary/SDL/All.h>

#include "renderer.h"

#include "canvas/layout.h"
#include "canvas/stroke.h"
#include "canvas/save.h"
#include "canvas/camera.h"

using namespace ctl;

void draw_textures(const Renderer &r, const CanvasContext &c)
{
	for (const auto &t : c.textures)
	{
		const auto world = c.cam.world_screen(mth::Rect{ t.dim.x, t.dim.y, t.dim.w, t.dim.h });
		r.draw_texture(t.data, world);
	}

	if (!c.target_line.points.empty())
		r.draw_texture(c.target_texture.data, c.target_texture.dim);
}

class Canvas
{
public:
	Canvas(Renderer &r)
	{
		r.set_stroke_radius(m_con.target_line.radius);
	}

	void draw(const Renderer &r)
	{
		draw_textures(r, m_con);
	}

	void event(const SDL_Event &e, const KeyEvent &ke, Window &w, Renderer &r)
	{
		handle_save_event(e, ke, w, m_con);
		handle_cam_event(e, ke, w, r, m_con);

		handle_stroke_event(e, ke, w, r, m_con);
		// m_con.strokes.event(e, ke, w, r, m_con.cam);
	}

private:
	CanvasContext m_con;
};
