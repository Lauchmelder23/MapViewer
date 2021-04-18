#include <osmobject.hpp>

#include <iostream>
#include <vector>

#include <tinyxml2.h>

#include <osmnode.hpp>
#include <osmway.hpp>
#include <osmrelation.hpp>

namespace xml = tinyxml2;

namespace osmp
{
	Object::Object(const std::string& file) :
		bounds({ 0.0f, 0.0f, 0.0f, 0.0f })
	{
		xml::XMLDocument doc;
		xml::XMLError result = doc.LoadFile(file.c_str());
		if (result != xml::XML_SUCCESS)
		{
			std::cerr << "Error: " << result << std::endl;
			return;
		}
	
		xml::XMLElement* root = doc.FirstChildElement();

		// Get bounds
		xml::XMLElement* bounds_elem = root->FirstChildElement("bounds");
		bounds = {
			GetSafeAttributeFloat(bounds_elem, "minlat"),
			GetSafeAttributeFloat(bounds_elem, "minlon"),
			GetSafeAttributeFloat(bounds_elem, "maxlat"),
			GetSafeAttributeFloat(bounds_elem, "maxlon")
		};

		// Get nodes
		xml::XMLElement* node_elem = root->FirstChildElement("node");
		while (node_elem != nullptr)
		{
			Node new_node = CreateNode(node_elem, this);
			nodes.insert(std::make_pair(new_node->id, new_node));

			node_elem = node_elem->NextSiblingElement("node");
		}

		// Get ways
		xml::XMLElement* way_elem = root->FirstChildElement("way");
		while (way_elem != nullptr)
		{
			Way new_way = CreateWay(way_elem, this);
			ways.insert(std::make_pair(new_way->id, new_way));

			way_elem = way_elem->NextSiblingElement("way");
		}

		// Get relations
		xml::XMLElement* relation_elem = root->FirstChildElement("relation");
		while (relation_elem != nullptr)
		{
			Relation new_way = CreateRelation(relation_elem, this);
			relations.insert(std::make_pair(new_way->id, new_way));

			relation_elem = relation_elem->NextSiblingElement("relation");
		}
	}

	Object::~Object()
	{

	}

	Nodes Object::GetNodes() const
	{
		Nodes vecNodes;
		for (std::map<uint64_t, Node>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
			vecNodes.push_back(it->second);

		return vecNodes;
	}

	size_t Object::GetNodesSize() const
	{
		return nodes.size();
	}

	Node Object::GetNode(uint64_t id) const
	{
		std::map<uint64_t, Node>::const_iterator node = nodes.find(id);
		if (node != nodes.end())
			return node->second;

		return nullptr;
	}

	Ways Object::GetWays() const
	{
		Ways vecWays;
		for (std::map<uint64_t, Way>::const_iterator it = ways.begin(); it != ways.end(); it++)
			vecWays.push_back(it->second);

		return vecWays;
	}

	size_t Object::GetWaysSize() const
	{
		return ways.size();
	}

	Way Object::GetWay(uint64_t id) const
	{
		std::map<uint64_t, Way>::const_iterator way = ways.find(id);
		if (way != ways.end())
			return way->second;

		return nullptr;
	}

	Relations Object::GetRelations() const
	{
		Relations vecRelations;
		for (std::map<uint64_t, Relation>::const_iterator it = relations.begin(); it != relations.end(); it++)
			vecRelations.push_back(it->second);

		return vecRelations;
	}

	size_t Object::GetRelationsSize() const
	{
		return relations.size();
	}

	Relation Object::GetRelation(uint64_t id) const
	{
		std::map<uint64_t, Relation>::const_iterator relation = relations.find(id);
		if (relation != relations.end())
			return relation->second;

		return nullptr;
	}
}