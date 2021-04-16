#pragma once
#include <vector>

#include "util.hpp"
#include <osmimember.hpp>
#include <osmtag.hpp>

namespace osmp
{
	class Object;

	class Node : public IMember
	{
	public:
		Node(const tinyxml2::XMLElement* xml, Object* parent);

	public:
		float lat, lon;
	};
}