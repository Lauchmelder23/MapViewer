#pragma once
#include <vector>

#include "util.hpp"

namespace osmp
{
	class Object;

	class Node 
	{
	public:
		Node(const tinyxml2::XMLElement* xml, Object* parent);

		const std::vector<Tag>& GetTags() const;
		size_t GetTagsSize() const;
		const Tag& GetTag(size_t index) const;

	private:
		Object* parent;

		std::vector<Tag> tags;

	public:
		unsigned int id;
		float lat, lon;
		std::string user;
		unsigned int uid;
		bool visible;
		std::string version;
		unsigned int changeset;
		std::string timestamp;
	};
}