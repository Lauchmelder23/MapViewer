#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>

#include <util.hpp>

namespace osmp
{
	class Object
	{
	public:
		explicit Object(const std::string& file);
		~Object();

		[[nodiscard]] Nodes GetNodes() const;
		[[nodiscard]] size_t GetNodesSize() const;
		[[nodiscard]] Node GetNode(uint64_t id) const;

		[[nodiscard]] Ways GetWays() const;
		[[nodiscard]] size_t GetWaysSize() const;
		[[nodiscard]] Way GetWay(uint64_t id) const;

		[[nodiscard]] Relations GetRelations() const;
		[[nodiscard]] size_t GetRelationsSize() const;
		[[nodiscard]] Relation GetRelation(uint64_t id) const;

	public:
		const std::string version;
		const std::string generator;

		Bounds bounds;
		
	private:
		std::map<uint64_t, Node> nodes;
		std::map<uint64_t, Way> ways;
		std::map<uint64_t, Relation> relations;
	};
}