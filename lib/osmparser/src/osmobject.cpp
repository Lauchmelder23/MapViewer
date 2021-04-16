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
			std::shared_ptr<Node> new_node = std::make_shared<Node>(node_elem, this);
			nodes.insert(std::make_pair(new_node->id, new_node));

			node_elem = node_elem->NextSiblingElement("node");
		}

		// Get ways
		xml::XMLElement* way_elem = root->FirstChildElement("way");
		while (way_elem != nullptr)
		{
			std::shared_ptr<Way> new_way = std::make_shared<Way>(way_elem, this);
			ways.insert(std::make_pair(new_way->id, new_way));

			way_elem = way_elem->NextSiblingElement("way");
		}

		// Get relations
		xml::XMLElement* relation_elem = root->FirstChildElement("relation");
		while (relation_elem != nullptr)
		{
			std::shared_ptr<Relation> new_way = std::make_shared<Relation>(relation_elem, this);
			relations.insert(std::make_pair(new_way->id, new_way));

			relation_elem = relation_elem->NextSiblingElement("relation");
		}
	}

	Object::~Object()
	{

	}

	std::vector<std::shared_ptr<Node>> Object::GetNodes() const
	{
		std::vector<std::shared_ptr<Node>> vecNodes;
		for (std::map<unsigned int, std::shared_ptr<Node>>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
			vecNodes.push_back(it->second);

		return vecNodes;
	}

	size_t Object::GetNodesSize() const
	{
		return nodes.size();
	}

	std::shared_ptr<Node> Object::GetNode(unsigned int id) const
	{
		std::map<unsigned int, std::shared_ptr<Node>>::const_iterator node = nodes.find(id);
		if (node != nodes.end())
			return node->second;

		return nullptr;
	}

	std::vector<std::shared_ptr<Way>> Object::GetWays() const
	{
		std::vector<std::shared_ptr<Way>> vecWays;
		for (std::map<unsigned int, std::shared_ptr<Way>>::const_iterator it = ways.begin(); it != ways.end(); it++)
			vecWays.push_back(it->second);

		return vecWays;
	}

	size_t Object::GetWaysSize() const
	{
		return ways.size();
	}

	std::shared_ptr<Way> Object::GetWay(unsigned int id) const
	{
		std::map<unsigned int, std::shared_ptr<Way>>::const_iterator way = ways.find(id);
		if (way != ways.end())
			return way->second;

		return nullptr;
	}

	std::vector<std::shared_ptr<Relation>> Object::GetRelations() const
	{
		std::vector<std::shared_ptr<Relation>> vecRelations;
		for (std::map<unsigned int, std::shared_ptr<Relation>>::const_iterator it = relations.begin(); it != relations.end(); it++)
			vecRelations.push_back(it->second);

		return vecRelations;
	}

	size_t Object::GetRelationsSize() const
	{
		return relations.size();
	}

	std::shared_ptr<Relation> Object::GetRelation(unsigned int id) const
	{
		std::map<unsigned int, std::shared_ptr<Relation>>::const_iterator relation = relations.find(id);
		if (relation != relations.end())
			return relation->second;

		return nullptr;
	}
}