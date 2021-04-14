#include <osmnode.hpp>

#include <tinyxml2.h>

namespace xml = tinyxml2;

namespace osmp
{
	Node::Node(const tinyxml2::XMLElement* node_elem, Object* parent) :
		parent(parent)
	{
		// Get Attribute
		id =		GetSafeAttributeUint(node_elem, "id");
		lat =		GetSafeAttributeFloat(node_elem, "lat");
		lon =		GetSafeAttributeFloat(node_elem, "lon");
		user =		GetSafeAttributeString(node_elem, "user");
		uid =		GetSafeAttributeUint(node_elem, "uid");
		visible =	GetSafeAttributeBool(node_elem, "visible");
		version =	GetSafeAttributeString(node_elem, "version");
		changeset = GetSafeAttributeUint(node_elem, "changeset");
		timestamp = GetSafeAttributeString(node_elem, "timestamp");

		const xml::XMLElement* tag_element = node_elem->FirstChildElement("tag");
		while (tag_element != nullptr)
		{
			tags.push_back({
				GetSafeAttributeString(tag_element, "k"),
				GetSafeAttributeString(tag_element, "v")
			});

			tag_element = tag_element->NextSiblingElement("tag");
		}
	}

	const std::vector<Tag>& Node::GetTags() const
	{
		return tags;
	}

	size_t Node::GetTagsSize() const
	{
		return tags.size();
	}

	const Tag& Node::GetTag(size_t index) const
	{
		return tags[index];
	}
}