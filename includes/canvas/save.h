#pragma once

#include "layout.h"
#include "event.h"
#include "pugixml.hpp"

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

void save(const char *filename, std::span<Line<float>> lines)
{
	pugi::xml_document doc;
	auto			   node = doc.append_child("doc");

	for (const auto &ls : lines)
	{
		auto lines_node					   = node.append_child("ls");
		lines_node.append_attribute("rad") = ls.radius;
		lines_node.append_attribute("col") = *(uint32_t *)&ls.color;

		for (const auto &l : ls.points)
		{
			auto line_node					= lines_node.append_child("l");
			line_node.append_attribute("x") = l.x;
			line_node.append_attribute("y") = l.y;
		}
	}

	doc.save_file(filename);
}

auto load(const char *filename) -> std::vector<Line<float>>
{
	pugi::xml_document doc;
	doc.load_file(filename);

	auto node = doc.first_child();

	std::vector<Line<float>> lines;

	for (auto ls = node.first_child(); ls != nullptr; ls = ls.next_sibling())
	{
		uint8_t radius = ls.attribute("rad").as_uint();

		auto	  c		= ls.attribute("col").as_uint();
		SDL_Color color = *(SDL_Color *)&c;

		std::vector<mth::Point<float>> ps;

		for (auto l = ls.first_child(); l != nullptr; l = l.next_sibling())
			ps.emplace_back(l.attribute("x").as_float(), l.attribute("y").as_float());

		lines.push_back({ radius, color, std::move(ps) });
	}

	return lines;
}
