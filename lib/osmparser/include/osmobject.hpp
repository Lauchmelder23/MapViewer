#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>

#include <util.hpp>

namespace osmp
{
	class Node;
	class Way;
	class Relation;

	class Object
	{
	public:
		Object(const std::string& file);
		~Object();

		std::vector<std::shared_ptr<Node>> GetNodes() const;
		size_t GetNodesSize() const;
		std::shared_ptr<Node> GetNode(unsigned int id) const;

		std::vector<std::shared_ptr<Way>> GetWays() const;
		size_t GetWaysSize() const;
		std::shared_ptr<Way> GetWay(unsigned int id) const;

		std::vector<std::shared_ptr<Relation>> GetRelations() const;
		size_t GetRelationsSize() const;
		std::shared_ptr<Relation> GetRelation(unsigned int id) const;

	public:
		const std::string version;
		const std::string generator;

		Bounds bounds;
		
	private:
		std::map<unsigned int, std::shared_ptr<Node>> nodes;
		std::map<unsigned int, std::shared_ptr<Way>> ways;
		std::map<unsigned int, std::shared_ptr<Relation>> relations;
	};
}