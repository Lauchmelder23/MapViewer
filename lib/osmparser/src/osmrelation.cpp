#include <osmrelation.hpp>

#include <memory>

#include <tinyxml2.h>
#include <osmobject.hpp>
#include <osmnode.hpp>
#include <osmway.hpp>

namespace xml = tinyxml2;

namespace osmp
{
	Relation::Relation(const xml::XMLElement* xml, Object* parent) :
		IMember(xml, parent, IMember::Type::RELATION), hasNullMembers(false)
	{
		const xml::XMLElement* member_element = xml->FirstChildElement("member");
		while (member_element != nullptr)
		{
			std::string memberType = GetSafeAttributeString(member_element, "type");
			unsigned int ref = GetSafeAttributeUint(member_element, "ref");
			std::string role = GetSafeAttributeString(member_element, "role");

			std::shared_ptr<IMember> member = nullptr;
			if (memberType == "node") {
				member = parent->GetNode(ref);
				nodes.push_back({ member, role });
			}
			else if (memberType == "way") {
				member = parent->GetWay(ref);
				if (member == nullptr) {
					hasNullMembers = true;
				}
				ways.push_back({ member, role });
			}

			member_element = member_element->NextSiblingElement("member");
		}
	}

	std::string Relation::GetRelationType()
	{
		return GetTag("type");
	}

	const std::vector<Relation::Member>& Relation::GetNodes() const
	{
		return nodes;
	}

	size_t Relation::GetNodesSize() const
	{
		return nodes.size();
	}

	const Relation::Member& Relation::GetNode(size_t index) const
	{
		return nodes[index];
	}

	const std::vector<Relation::Member>& Relation::GetWays() const
	{
		return ways;
	}

	size_t Relation::GetWaysSize() const
	{
		return ways.size();
	}

	const Relation::Member& Relation::GetWay(size_t index) const
	{
		return ways[index];
	}
}