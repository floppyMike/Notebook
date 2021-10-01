#pragma once

#include <optional>
#include "renderer/sdl2.h"

// clang-format off
template<typename T>
concept is_renderer = requires(T t)
{
	typename T::Texture;
	typename T::Font;

	t.set_draw_color(SDL_Color{});
	t.set_render_target(typename T::Texture());

	t.render_target();

	t.draw_stroke(mth::Point<int>(), mth::Point<int>());

	{ t.get_texture_size(typename T::Texture()) } -> std::same_as<mth::Dim<int>>;
	
	{ t.create_texture(int(), int()) } -> std::same_as<typename T::Texture>;
	{ t.create_font("", int()) } -> std::same_as<std::optional<typename T::Font>>;
	{ t.create_text(typename T::Font(), "") } -> std::same_as<typename T::Texture>;

	t.draw_texture(typename T::Texture(), mth::Rect<int>());
	t.draw_frame(typename T::Texture(), mth::Rect<int>(), mth::Rect<int>());
	t.draw_rect(mth::Rect<int>());
	t.draw_rectfilled(mth::Rect<int>());
	t.draw_line(mth::Point<int>(), mth::Point<int>());

	{ t.crop_texture(typename T::Texture(), mth::Rect<int>()) } -> std::same_as<typename T::Texture>;
	{ t.load_bmp("") } -> std::same_as<std::optional<typename T::Texture>>;
	t.refresh();

	t.render((void (*)())nullptr);
};
// clang-format on

static_assert(is_renderer<Renderer>);
