#include <osmnode.hpp>

#include <tinyxml2.h>

namespace xml = tinyxml2;

namespace osmp
{
	Node::Node(const tinyxml2::XMLElement* node_elem, Object* parent) :
		IMember(node_elem, parent, IMember::Type::NODE)
	{
		// Get Attribute
		lat =		GetSafeAttributeFloat(node_elem, "lat");
		lon =		GetSafeAttributeFloat(node_elem, "lon");
	}
}