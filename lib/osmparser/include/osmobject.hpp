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
		std::shared_ptr<Node> GetNode(uint64_t id) const;

		std::vector<std::shared_ptr<Way>> GetWays() const;
		size_t GetWaysSize() const;
		std::shared_ptr<Way> GetWay(uint64_t id) const;

		std::vector<std::shared_ptr<Relation>> GetRelations() const;
		size_t GetRelationsSize() const;
		std::shared_ptr<Relation> GetRelation(uint64_t id) const;

	public:
		const std::string version;
		const std::string generator;

		Bounds bounds;
		
	private:
		std::map<uint64_t, std::shared_ptr<Node>> nodes;
		std::map<uint64_t, std::shared_ptr<Way>> ways;
		std::map<uint64_t, std::shared_ptr<Relation>> relations;
	};
}