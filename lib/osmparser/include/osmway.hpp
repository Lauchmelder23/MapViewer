#pragma once
#include <vector>
#include <memory>

#include <util.hpp>
#include <osmtag.hpp>
#include <osmimember.hpp>

namespace osmp
{
	class Object;
	class Node;

	class Way : public IMember
	{
	public:
		Way(const tinyxml2::XMLElement* way_elem, Object* parent);

		const std::vector<std::shared_ptr<Node>>& GetNodes() const;
		size_t GetNodesSize() const;
		const std::shared_ptr<Node>& GetNode(size_t index) const;

	public:
		bool area, closed;	// Closed := Startpoint = endpoint, Area := Closed AND certain conditions are not met

	private:
		std::vector<std::shared_ptr<Node>> nodes;
	};
}