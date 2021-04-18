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

	[[nodiscard]] std::string GetSafeAttributeString(const tinyxml2::XMLElement* elem, const std::string& name);
	[[nodiscard]] double GetSafeAttributeFloat(const tinyxml2::XMLElement* elem, const std::string& name);
	[[nodiscard]] uint64_t GetSafeAttributeUint64(const tinyxml2::XMLElement* elem, const std::string& name);
	[[nodiscard]] bool GetSafeAttributeBool(const tinyxml2::XMLElement* elem, const std::string& name);
}