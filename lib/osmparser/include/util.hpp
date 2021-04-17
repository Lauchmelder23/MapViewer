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
		double minlat, minlon, maxlat, maxlon;
	} Bounds;

	std::string GetSafeAttributeString(const tinyxml2::XMLElement* elem, const std::string& name);
	double GetSafeAttributeFloat(const tinyxml2::XMLElement* elem, const std::string& name);
	uint64_t GetSafeAttributeUint64(const tinyxml2::XMLElement* elem, const std::string& name);
	bool GetSafeAttributeBool(const tinyxml2::XMLElement* elem, const std::string& name);
}