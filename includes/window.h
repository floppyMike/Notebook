#pragma once

#include <SDL.h>
#include "renderer.h"

using namespace ctl;

struct WindowContext
{
	sdl::Window win;
};

class Window
{
public:
	void init()
	{
		m_con.win.reset(SDL_CreateWindow("Notetaker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
										 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

		if (!m_con.win)
			throw std::runtime_error(SDL_GetError());
	}

	auto create_renderer() const -> Renderer
	{
		assert(m_con.win);

		Renderer r;
		r.init(m_con.win.get());
		return r;
	}

	auto get_windowsize() const -> mth::Dim<int>
	{
		assert(m_con.win);

		mth::Dim<int> dim;
		SDL_GetWindowSize(m_con.win.get(), &dim.w, &dim.h);

		return dim;
	}

	auto get_windowid() const -> uint32_t
	{
		assert(m_con.win);
		return SDL_GetWindowID(m_con.win.get());
	}

private:
	WindowContext m_con;
};

#ifdef __linux__

inline auto open_file_load() -> std::optional<std::string>
{
	char  filename[1024];
	FILE *f = popen("zenity --file-selection --title='Choose file to load...'", "r");
	fgets(filename, 1024, f);

	int ret = pclose(f);

	if (ret != 0)
		return std::nullopt;

	std::string s = filename;
	s.pop_back(); // Why

	return s;
}

inline auto open_file_save() -> std::optional<std::string>
{
	char  filename[1024];
	FILE *f = popen("zenity --file-selection --save --title='Choose a path to save to...'", "r");
	fgets(filename, 1024, f);

	int ret = pclose(f);

	if (ret != 0)
		return std::nullopt;

	std::string s = filename;
	s.pop_back(); // Why

	return s;
}

#elif _WIN32

#define NOMINMAX
#include <windows.h>

inline auto open_file_load() -> std::optional<std::string>
{
	char filename[1024] = { 0 };
	
	OPENFILENAME ofn = { .lStructSize = sizeof ofn, .lpstrFile = filename, .nMaxFile = 1024, .lpstrTitle = "Choose file to load..." };

	if (GetOpenFileName(&ofn) == 0)
		return std::nullopt;
	
	return std::string(filename);
}

inline auto open_file_save() -> std::optional<std::string>
{
	char filename[1024] = { 0 };
	
	OPENFILENAME ofn = { .lStructSize = sizeof ofn, .lpstrFile = filename, .nMaxFile = 1024, .lpstrTitle = "Choose a path to save to..." };

	if (GetSaveFileName(&ofn) == 0)
		return std::nullopt;
	
	return std::string(filename);
}

#endif
