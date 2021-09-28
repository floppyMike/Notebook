#pragma once

#include <array>
#include <optional>

#include <CustomLibrary/IO.h>
#include <CustomLibrary/utility.h>

#include "layout.h"
#include "event.h"
#include "window.h"
#include "pugixml.hpp"

// -----------------------------------------------------------------------------
// Saving
// -----------------------------------------------------------------------------

/**
 * @brief Save the stroke component to the xml doc
 *
 * @param c Get the stroke items
 * @param node Location to store the xml data
 *
 * @return
 */
inline void save_strokes(const CanvasContext &c, pugi::xml_node &node)
{
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
}

/**
 * @brief Save the text component to the xml doc
 *
 * @param c Get the text info for storage
 * @param node Xml document node to store to
 *
 * @return
 */
inline auto save_text(const CanvasContext &c, pugi::xml_node &node)
{
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
}

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
inline void save(const CanvasContext &c, const char *filename)
{
	static_assert(sizeof(SDL_Color) == 4, "SDL_Color must be 4 bytes long.");

	pugi::xml_document doc;

	auto node = doc.append_child("doc");

	save_strokes(c, node);
	save_text(c, node);

	doc.save_file(filename);
}

// -----------------------------------------------------------------------------
// Loading
// -----------------------------------------------------------------------------

/**
 * @brief Load attributes with exception
 *
 * @param ls node to load from
 * @param ids attributes as string array
 *
 * @return xml_attributes
 */
template<size_t n>
auto load_attributes(const pugi::xml_node &ls, std::array<const char *, n> ids)
	-> std::optional<std::array<pugi::xml_attribute, n>>
{
	std::array<pugi::xml_attribute, n> attribs;
	std::generate(attribs.begin(), attribs.end(), [p = ids.begin(), &ls]() mutable { return ls.attribute(*p++); });

	if (!std::none_of(attribs.begin(), attribs.end(), [](const auto &i) { return i.empty(); }))
		return std::nullopt;

	return attribs;
}

inline void load_strokes(CanvasContext &c, pugi::xml_node &node)
{
	for (auto ls = node.child("line").first_child(); ls != nullptr; ls = ls.next_sibling())
	{
		const auto attrib = load_attributes(ls, std::array{ "r", "c", "s", "x", "y", "w", "h" });

		if (!attrib)
			throw std::runtime_error("A line has incomplete attributes.");

		const auto &nodes = attrib.value();

		const auto radius = nodes[0].as_float();
		const auto color  = (SDL_Color &)ctl::unmove(nodes[1].as_uint());
		const auto scale  = nodes[2].as_float();

		std::vector<mth::Point<float>> ps;

		for (auto l = ls.first_child(); l != nullptr; l = l.next_sibling())
		{
			const auto attrib = load_attributes(l, std::array{ "x", "y" });

			if (!attrib)
				throw std::runtime_error("Coords missing for a line.");

			const auto &nodes = attrib.value();

			ps.push_back({ nodes[0].as_float(), nodes[1].as_float() });
		}

		c.swts.push_back(
			{ .dim = { nodes[3].as_float(), nodes[4].as_float(), nodes[5].as_float(), nodes[6].as_float() } });

		c.swls.push_back({ std::move(ps) });
		c.swlis.push_back({ radius, scale, color });
	}
}

inline void load_text(CanvasContext &c, pugi::xml_node &node)
{
	for (auto t = node.child("text").first_child(); t != nullptr; t = t.next_sibling())
	{
		const auto attrib = load_attributes(t, std::array{ "s", "t", "x", "y" });

		if (!attrib)
			throw std::runtime_error("A text has incomplete attributes.");

		const auto &nodes = attrib.value();

		const auto scale = nodes[0].as_float();
		const auto text	 = nodes[1].as_string();

		const auto point = mth::Point<float>{ nodes[2].as_float(), nodes[3].as_float() };

		c.txwtxis.push_back({ .str = text, .scale = scale });
		c.txwts.push_back({ .dim = { point.x, point.y, 0.F, 0.F } });
	}
}

/**
 * @brief Load save.xml into lines info and texture dimensions
 *
 * @param c Place to load the stored information
 */
inline void load(CanvasContext &c, const char *filename)
{
	static_assert(sizeof(SDL_Color) == 4, "SDL_Color must be 4 bytes long.");

	pugi::xml_document doc;

	auto status = doc.load_file(filename);

	if (status.status != pugi::status_ok) // Handle parse error
		throw std::runtime_error(status.description());

	auto node = doc.first_child();

	load_strokes(c, node);
	load_text(c, node);
}
