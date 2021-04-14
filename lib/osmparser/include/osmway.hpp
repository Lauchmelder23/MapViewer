#pragma once
#include <vector>

#include <util.hpp>

namespace osmp
{
	class Object;
	class Node;

	class Way
	{
	public:
		Way(const tinyxml2::XMLElement* way_elem, Object* parent);

		const std::vector<Tag>& GetTags() const;
		size_t GetTagsSize() const;
		const Tag& GetTag(size_t index) const;

		const std::vector<const Node*>& GetNodes() const;
		size_t GetNodesSize() const;
		const Node& GetNode(size_t index) const;

	public:
		unsigned int id;
		std::string user;
		unsigned int uid;
		bool visible;
		std::string version;
		unsigned int changeset;
		std::string timestamp;

	private:
		Object* parent;

		std::vector<const Node*> nodes;
		std::vector<Tag> tags;
	};
}