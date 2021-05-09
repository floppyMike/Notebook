#pragma once

#include <bitset>

enum KeyEventMap
{
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,

	ALL,
};

using KeyEvent = std::bitset<KeyEventMap::ALL>;
