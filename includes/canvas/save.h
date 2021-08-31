#pragma once

#include "layout.h"
#include "event.h"
#include "pugixml.hpp"

/**
 * @brief Save the canvas into save.xml using XML
 * <doc>
 *     <ls rad= col= s= x= y= w= h= >
 *         <l x= y= > ...
 *     </ls> ...
 * </doc>
 *
 * @param c Get lines info and texture dimensions
 */
void save(const CanvasContext &c)
{
	static_assert(sizeof(SDL_Color) == 4, "SDL_Color must be 4 bytes long.");

	pugi::xml_document doc;
	auto			   node = doc.append_child("doc");

	for (size_t i = 0; i < c.textures.size(); ++i)
	{
		const auto &texture	  = c.textures[i];
		const auto &line	  = c.lines[i];
		const auto &line_info = c.lines_info[i];

		auto lines_node = node.append_child("ls");

		lines_node.append_attribute("r") = line_info.radius;
		lines_node.append_attribute("c") = *(uint32_t *)&line_info.color;
		lines_node.append_attribute("s") = line_info.scale;

		lines_node.append_attribute("x") = texture.dim.x;
		lines_node.append_attribute("y") = texture.dim.y;
		lines_node.append_attribute("w") = texture.dim.w;
		lines_node.append_attribute("h") = texture.dim.h;

		for (const auto &l : line.points)
		{
			auto line_node					= lines_node.append_child("l");
			line_node.append_attribute("x") = l.x;
			line_node.append_attribute("y") = l.y;
		}
	}

	doc.save_file("save.xml");
}

/**
 * @brief Load save.xml into lines info and texture dimensions
 *
 * @param c Place to load the stored information
 */
inline void load(CanvasContext &c)
{
	static_assert(sizeof(SDL_Color) == 4, "SDL_Color must be 4 bytes long.");

	pugi::xml_document doc;
	doc.load_file("save.xml");

	auto node = doc.first_child();

	for (auto ls = node.first_child(); ls != nullptr; ls = ls.next_sibling())
	{
		const float		radius	  = ls.attribute("r").as_float();
		const auto		color_int = ls.attribute("c").as_uint();
		const SDL_Color color	  = *(const SDL_Color *)&color_int;
		const float		scale	  = ls.attribute("s").as_float();

		c.textures.push_back({ .dim = { ls.attribute("x").as_float(), ls.attribute("y").as_float(),
										ls.attribute("w").as_float(), ls.attribute("h").as_float() } });

		std::vector<mth::Point<float>> ps;
		for (auto l = ls.first_child(); l != nullptr; l = l.next_sibling())
			ps.push_back({ l.attribute("x").as_float(), l.attribute("y").as_float() });

		c.lines.push_back({ std::move(ps) });
		c.lines_info.push_back({ radius, scale, color });
	}
}
