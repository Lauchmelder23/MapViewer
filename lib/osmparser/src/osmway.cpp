#include <osmway.hpp>

#include <string>

#include <tinyxml2.h>
#include <osmobject.hpp>
#include <osmtag.hpp>

namespace xml = tinyxml2;

namespace osmp
{
	Way::Way(const tinyxml2::XMLElement* way_elem, Object* parent) :
		IMember(way_elem, parent, IMember::Type::WAY)
	{
		area = GetSafeAttributeBool(way_elem, "area");
		closed = false;

		const xml::XMLElement* nd_elem = way_elem->FirstChildElement("nd");
		while (nd_elem != nullptr)
		{
			nodes.push_back(
				parent->GetNode(GetSafeAttributeUint64(nd_elem, "ref"))
			);

			nd_elem = nd_elem->NextSiblingElement("nd");
		}
		
		if (nodes.front() == nodes.back())
		{
			closed = true;

			if (!area && GetTag("barrier") == "" && GetTag("highway") == "")	// this code sucks, it can be done better
				area = true;
		}
	}

	const std::vector<std::shared_ptr<Node>>& Way::GetNodes() const
	{
		return nodes;
	}

	size_t Way::GetNodesSize() const
	{
		return nodes.size();
	}

	const std::shared_ptr<Node>& Way::GetNode(size_t index) const
	{
		return nodes[index];
	}
}