#include <osmobject.hpp>

#include <iostream>
#include <vector>

#include <tinyxml2.h>

#include <osmnode.hpp>
#include <osmway.hpp>

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
			Node* new_node = new Node(node_elem, this);
			nodes.insert(std::make_pair(new_node->id, new_node));

			node_elem = node_elem->NextSiblingElement("node");
		}

		// Get ways
		xml::XMLElement* way_elem = root->FirstChildElement("way");
		while (way_elem != nullptr)
		{
			Way* new_way = new Way(way_elem, this);
			ways.insert(std::make_pair(new_way->id, new_way));

			way_elem = way_elem->NextSiblingElement("way");
		}
	}

	Object::~Object()
	{
		for (std::map<unsigned int, Way*>::iterator it = ways.begin(); it != ways.end(); ++it)
		{
			delete it->second;
		}
		ways.clear();

		for (std::map<unsigned int, Node*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			delete it->second;
		}
		nodes.clear();
	}

	std::vector<Node*> Object::GetNodes() const
	{
		std::vector<Node*> vecNodes;
		for (std::map<unsigned int, Node*>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
			vecNodes.push_back(it->second);

		return vecNodes;
	}

	size_t Object::GetNodesSize() const
	{
		return nodes.size();
	}

	const Node* Object::GetNode(unsigned int id) const
	{
		std::map<unsigned int, Node*>::const_iterator node = nodes.find(id);
		if (node != nodes.end())
			return node->second;

		return nullptr;
	}

	std::vector<Way*> Object::GetWays() const
	{
		std::vector<Way*> vecWays;
		for (std::map<unsigned int, Way*>::const_iterator it = ways.begin(); it != ways.end(); it++)
			vecWays.push_back(it->second);

		return vecWays;
	}

	size_t Object::GetWaysSize() const
	{
		return ways.size();
	}

	const Way* Object::GetWay(unsigned int id) const
	{
		std::map<unsigned int, Way*>::const_iterator way = ways.find(id);
		if (way != ways.end())
			return way->second;

		return nullptr;
	}
}