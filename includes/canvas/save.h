#pragma once

#include "layout.h"

void save(const CanvasContext &c)
{
	pugi::xml_document doc;
	auto			   node = doc.append_child("doc");

	// c.strokes.save(node);

	doc.save_file("save.xml");
}

void load(CanvasContext &c)
{
	pugi::xml_document doc;
	doc.load_file("save.xml");

	auto node = doc.first_child();

	// c->strokes.load(node);
}
