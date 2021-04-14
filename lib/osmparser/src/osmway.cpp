#include <osmway.hpp>

#include <tinyxml2.h>
#include <osmobject.hpp>

namespace xml = tinyxml2;

namespace osmp
{
	Way::Way(const tinyxml2::XMLElement* way_elem, Object* parent) :
		parent(parent)
	{
		// Attributes
		id =		GetSafeAttributeUint(way_elem, "id");
		user =		GetSafeAttributeString(way_elem, "user");
		uid =		GetSafeAttributeUint(way_elem, "uid");
		visible =	GetSafeAttributeBool(way_elem, "visible");
		version =	GetSafeAttributeString(way_elem, "version");
		changeset = GetSafeAttributeUint(way_elem, "changeset");
		timestamp = GetSafeAttributeString(way_elem, "timestamp");

		const xml::XMLElement* nd_elem = way_elem->FirstChildElement("nd");
		while (nd_elem != nullptr)
		{
			nodes.push_back(
				parent->GetNode(GetSafeAttributeUint(nd_elem, "ref"))
			);

			nd_elem = nd_elem->NextSiblingElement("nd");
		}

		const xml::XMLElement* tag_elem = way_elem->FirstChildElement("tag");
		while (tag_elem != nullptr)
		{
			tags.push_back({
				GetSafeAttributeString(tag_elem, "k"),
				GetSafeAttributeString(tag_elem, "v")
			});

			tag_elem = tag_elem->NextSiblingElement("tag");
		}
	}

	const std::vector<Tag>& Way::GetTags() const
	{
		return tags;
	}

	size_t Way::GetTagsSize() const
	{
		return tags.size();
	}

	const Tag& Way::GetTag(size_t index) const
	{
		return tags[index];
	}

	const std::vector<const Node*>& Way::GetNodes() const
	{
		return nodes;
	}

	size_t Way::GetNodesSize() const
	{
		return nodes.size();
	}

	const Node& Way::GetNode(size_t index) const
	{
		return *(nodes[index]);
	}
}