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
		explicit Object(const std::string& file);
		~Object();

		[[nodiscard]] std::vector<std::shared_ptr<Node>> GetNodes() const;
		[[nodiscard]] size_t GetNodesSize() const;
		[[nodiscard]] std::shared_ptr<Node> GetNode(uint64_t id) const;

		[[nodiscard]] std::vector<std::shared_ptr<Way>> GetWays() const;
		[[nodiscard]] size_t GetWaysSize() const;
		[[nodiscard]] std::shared_ptr<Way> GetWay(uint64_t id) const;

		[[nodiscard]] std::vector<std::shared_ptr<Relation>> GetRelations() const;
		[[nodiscard]] size_t GetRelationsSize() const;
		[[nodiscard]] std::shared_ptr<Relation> GetRelation(uint64_t id) const;

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