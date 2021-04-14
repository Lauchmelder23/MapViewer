#pragma once
#include <string>
#include <map>
#include <vector>

#include <util.hpp>

namespace osmp
{
	class Node;
	class Way;

	class Object
	{
	public:
		Object(const std::string& file);
		~Object();

		std::vector<Node*> GetNodes() const;
		size_t GetNodesSize() const;
		const Node* GetNode(unsigned int id) const;

		std::vector<Way*> GetWays() const;
		size_t GetWaysSize() const;
		const Way* GetWay(unsigned int id) const;

	public:
		const std::string version;
		const std::string generator;

		Bounds bounds;
		
	private:
		std::map<unsigned int, Node*> nodes;
		std::map<unsigned int, Way*> ways;
	};
}