#pragma once

#include <CustomLibrary/IO.h>

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
inline void save(const CanvasContext &c)
{
	static_assert(sizeof(SDL_Color) == 4, "SDL_Color must be 4 bytes long.");

	pugi::xml_document doc;
	auto			   node = doc.append_child("doc");

	auto lines = node.append_child("line");

	for (size_t i = 0; i < c.swts.size(); ++i)
	{
		const auto &t  = c.swts[i];
		const auto &l  = c.swls[i];
		const auto &li = c.swlis[i];

		auto ln = lines.append_child("l");

		ln.append_attribute("r") = li.radius;
		ln.append_attribute("c") = *(uint32_t *)&li.color;
		ln.append_attribute("s") = li.scale;

		ln.append_attribute("x") = t.dim.x;
		ln.append_attribute("y") = t.dim.y;
		ln.append_attribute("w") = t.dim.w;
		ln.append_attribute("h") = t.dim.h;

		for (const auto &l : l.points)
		{
			auto subnode = ln.append_child("p");

			subnode.append_attribute("x") = l.x;
			subnode.append_attribute("y") = l.y;
		}
	}

	auto texts = node.append_child("text");

	for (size_t i = 0; i < c.txwts.size(); ++i)
	{
		const auto &t	= c.txwts[i];
		const auto &txi = c.txwtxis[i];

		auto ln = texts.append_child("t");

		ln.append_attribute("s") = txi.scale;
		ln.append_attribute("t") = txi.str.c_str();

		ln.append_attribute("x") = t.dim.x;
		ln.append_attribute("y") = t.dim.y;
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

	auto node = doc.first_child().first_child();

	for (auto ls = node.first_child(); ls != nullptr; ls = ls.next_sibling())
	{
		const float		radius	  = ls.attribute("r").as_float();
		const auto		color_int = ls.attribute("c").as_uint();
		const SDL_Color color	  = *(const SDL_Color *)&color_int;
		const float		scale	  = ls.attribute("s").as_float();

		c.swts.push_back({ .dim = { ls.attribute("x").as_float(), ls.attribute("y").as_float(),
									ls.attribute("w").as_float(), ls.attribute("h").as_float() } });

		std::vector<mth::Point<float>> ps;
		for (auto l = ls.first_child(); l != nullptr; l = l.next_sibling())
			ps.push_back({ l.attribute("x").as_float(), l.attribute("y").as_float() });

		c.swls.push_back({ std::move(ps) });
		c.swlis.push_back({ radius, scale, color });
	}

	node = node.next_sibling();

	for (auto t = node.first_child(); t != nullptr; t = t.next_sibling())
	{
		const auto scale = t.attribute("s").as_float();
		const auto text	 = t.attribute("t").as_string();
		const auto point = mth::Point<float>{ t.attribute("x").as_float(), t.attribute("y").as_float() };

		c.txwtxis.push_back({ .str = text, .scale = scale });
		c.txwts.push_back({ .dim = { point.x, point.y, 0.F, 0.F } });
	}
}
