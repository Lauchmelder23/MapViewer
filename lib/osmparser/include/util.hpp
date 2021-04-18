#pragma once

#include <string>
#include <vector>
#include <memory>

namespace tinyxml2
{
	class XMLElement;
}

namespace osmp
{
	class INode;
	class IWay;
	class IRelation;

	typedef std::shared_ptr<INode>		Node;
	typedef std::shared_ptr<IWay>		Way;
	typedef std::shared_ptr<IRelation>	Relation;

	typedef std::vector<Node>			Nodes;
	typedef std::vector<Way>			Ways;
	typedef std::vector<Relation>		Relations;

	typedef struct sBounds 
	{
		double minlat, minlon, maxlat, maxlon;
	} Bounds;

	[[nodiscard]] std::string GetSafeAttributeString(const tinyxml2::XMLElement* elem, const std::string& name);
	[[nodiscard]] double GetSafeAttributeFloat(const tinyxml2::XMLElement* elem, const std::string& name);
	[[nodiscard]] uint64_t GetSafeAttributeUint64(const tinyxml2::XMLElement* elem, const std::string& name);
	[[nodiscard]] bool GetSafeAttributeBool(const tinyxml2::XMLElement* elem, const std::string& name);
}