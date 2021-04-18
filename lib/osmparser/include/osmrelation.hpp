#pragma once
#include <vector>
#include <memory>

#include <util.hpp>
#include <osmtag.hpp>
#include <osmimember.hpp>

namespace osmp
{
	class Object;

	class Relation : public IMember
	{
	public:
		typedef struct sMember {
			std::shared_ptr<IMember> member;
			std::string role;
		} Member;

	public:
		Relation(const tinyxml2::XMLElement* xml, Object* parent);

		[[nodiscard]] std::string GetRelationType();

		[[nodiscard]] const std::vector<Member>& GetNodes() const;
		[[nodiscard]] size_t GetNodesSize() const;
		[[nodiscard]] const Member& GetNode(size_t index) const;

		[[nodiscard]] const std::vector<Member>& GetWays() const;
		[[nodiscard]] size_t GetWaysSize() const;
		[[nodiscard]] const Member& GetWay(size_t index) const;

		[[nodiscard]] bool HasNullMembers() const { return hasNullMembers; }

	private:
		std::string relationType;
		bool hasNullMembers;

		std::vector<Member> nodes;
		std::vector<Member> ways;
	};
}