#pragma once

#include <optional>
#include "renderer/sdl2.h"

// clang-format off
template<typename T>
concept is_renderer = requires(T t)
{
	t.set_draw_color(SDL_Color{});
	t.set_render_target(TextureData());
	t.set_stroke_radius(int());

	t.render_target();

	t.draw_joint_stroke(mth::Point<int>(), int());
	t.draw_con_stroke(mth::Point<int>(), mth::Point<int>(), int());

	{ t.get_texture_size(TextureData()) } -> std::same_as<mth::Dim<int>>;
	
	{ t.create_texture(int(), int()) } -> std::same_as<TextureData>;
	{ t.create_font("", int()) } -> std::same_as<std::optional<FontData>>;
	{ t.create_text(FontData(), "") } -> std::same_as<TextureData>;

	t.draw_texture(TextureData(), mth::Rect<int>());
	t.draw_frame(TextureData(), mth::Rect<int>(), mth::Rect<int>());
	t.draw_rect(mth::Rect<int>());
	t.draw_rectfilled(mth::Rect<int>());
	t.draw_line(mth::Point<int>(), mth::Point<int>());

	{ t.crop_texture(TextureData(), mth::Rect<int>()) } -> std::same_as<TextureData>;
	{ t.load_bmp("") } -> std::same_as<std::optional<TextureData>>;
	t.refresh();

	t.render((void (*)())nullptr);
};
// clang-format on

static_assert(is_renderer<Renderer>);
