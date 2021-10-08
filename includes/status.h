#pragma once

enum class CanvasStatus
{
	PAINTING,
	TYPING,
	SELECTING,
};

enum Event
{
	EVENT_SELECT,
	EVENT_DRAW,
	EVENT_QUICKSAVE,
	EVENT_SAVE,
	EVENT_LOAD,
};
