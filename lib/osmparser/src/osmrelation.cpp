#include "..\include\osmrelation.hpp"
#include <osmrelation.hpp>

#include <memory>

#include <tinyxml2.h>
#include <osmobject.hpp>
#include <osmnode.hpp>
#include <osmway.hpp>

namespace xml = tinyxml2;

namespace osmp
{
	IRelation::IRelation(const xml::XMLElement* xml, Object* parent) :
		IMember(xml, parent, IMember::Type::RELATION), hasNullMembers(false)
	{
		const xml::XMLElement* member_element = xml->FirstChildElement("member");
		while (member_element != nullptr)
		{
			std::string memberType = GetSafeAttributeString(member_element, "type");
			uint64_t ref = GetSafeAttributeUint64(member_element, "ref");
			std::string role = GetSafeAttributeString(member_element, "role");

			if (memberType == "node") {
				Node node = parent->GetNode(ref);
				nodes.push_back({ node, role });
			}
			else if (memberType == "way") {
				Way way = parent->GetWay(ref);
				if (way == nullptr) {
					hasNullMembers = true;
				}
				ways.push_back({ way, role });
			}

			member_element = member_element->NextSiblingElement("member");
		}
	}

	namespace {
		struct ConcreteRelation : public IRelation {
			ConcreteRelation(const tinyxml2::XMLElement* way_elem, Object* parent) :
				IRelation(way_elem, parent)
			{}
		};
	}

	Relation CreateRelation(const tinyxml2::XMLElement* xml, Object* parent)
	{
		return std::make_shared<ConcreteRelation>(xml, parent);
	}

	std::string IRelation::GetRelationType() const
	{
		return GetTag("type");
	}

	const MemberNodes& IRelation::GetNodes() const
	{
		return nodes;
	}

	size_t IRelation::GetNodesSize() const
	{
		return nodes.size();
	}

	const MemberNode& IRelation::GetNode(size_t index) const
	{
		return nodes[index];
	}

	const MemberWays& IRelation::GetWays() const
	{
		return ways;
	}

	size_t IRelation::GetWaysSize() const
	{
		return ways.size();
	}

	const MemberWay& IRelation::GetWay(size_t index) const
	{
		return ways[index];
	}
}