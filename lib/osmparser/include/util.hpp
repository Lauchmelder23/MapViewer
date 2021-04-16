#pragma once

#include <string>

namespace tinyxml2
{
	class XMLElement;
}

namespace osmp
{
	typedef struct sBounds 
	{
		float minlat, minlon, maxlat, maxlon;
	} Bounds;

	std::string GetSafeAttributeString(const tinyxml2::XMLElement* elem, const std::string& name);
	float GetSafeAttributeFloat(const tinyxml2::XMLElement* elem, const std::string& name);
	unsigned int GetSafeAttributeUint(const tinyxml2::XMLElement* elem, const std::string& name);
	bool GetSafeAttributeBool(const tinyxml2::XMLElement* elem, const std::string& name);
}