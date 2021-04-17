#include <osmimember.hpp>

#include <osmobject.hpp>
#include <tinyxml2.h>

namespace xml = tinyxml2;

namespace osmp
{
	IMember::IMember(const xml::XMLElement* element, Object* parent, IMember::Type type) :
		type(type), parent(parent)
	{
		// Get Attribute
		id = GetSafeAttributeUint64(element, "id");
		user = GetSafeAttributeString(element, "user");
		uid = GetSafeAttributeUint64(element, "uid");
		visible = GetSafeAttributeBool(element, "visible");
		version = GetSafeAttributeString(element, "version");
		changeset = GetSafeAttributeUint64(element, "changeset");
		timestamp = GetSafeAttributeString(element, "timestamp");

		const xml::XMLElement* tag_element = element->FirstChildElement("tag");
		while (tag_element != nullptr)
		{
			tags.push_back({
				GetSafeAttributeString(tag_element, "k"),
				GetSafeAttributeString(tag_element, "v"),
				});

			tag_element = tag_element->NextSiblingElement("tag");
		}
	}

	IMember::Type IMember::GetType() const
	{
		return type;
	}

	const std::vector<Tag>& IMember::GetTags() const
	{
		return tags;
	}

	size_t IMember::GetTagsSize() const
	{
		return tags.size();
	}

	const Tag& IMember::GetTag(size_t index) const
	{
		return tags[index];
	}

	std::string IMember::GetTag(const std::string& key) const
	{
		for (const Tag& tag : tags)
		{
			if (tag.k == key)
			{
				return tag.v;
			}
		}

		return "";
	}
}
