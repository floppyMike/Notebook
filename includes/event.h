#pragma once

#include <bitset>

enum class Status
{
	PAINTING,
	TYPING,
};

enum KeyEventMap
{
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,

	ALL,
};

using KeyEvent = std::bitset<KeyEventMap::ALL>;

enum Event
{
	EVENT_DRAW,
	EVENT_SELECT,
	EVENT_TYPE,
	EVENT_SAVE,
	EVENT_LOAD,

	EVENT_PRESS_DOWN,
	EVENT_PRESS_UP,
};
