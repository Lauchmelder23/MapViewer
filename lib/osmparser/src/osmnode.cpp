#include <osmnode.hpp>

#include <tinyxml2.h>

namespace xml = tinyxml2;

namespace osmp
{
	INode::INode(const tinyxml2::XMLElement* node_elem, Object* parent) :
		IMember(node_elem, parent, IMember::Type::NODE)
	{
		// Get Attribute
		lat =		GetSafeAttributeFloat(node_elem, "lat");
		lon =		GetSafeAttributeFloat(node_elem, "lon");
	}


	namespace {
		struct ConcreteNode : public INode {
			ConcreteNode(const tinyxml2::XMLElement* node_elem, Object* parent) :
				INode(node_elem, parent)
			{}
		};
	}

	Node CreateNode(const tinyxml2::XMLElement* node_elem, Object* parent)
	{
		return std::make_shared<ConcreteNode>(node_elem, parent);
	}
}