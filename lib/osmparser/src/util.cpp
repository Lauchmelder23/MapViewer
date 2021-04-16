#include <util.hpp>

#include <iostream>
#include <tinyxml2.h>

namespace xml = tinyxml2;

namespace osmp
{
#define FAILED(err) (err != xml::XML_SUCCESS)

	std::string GetSafeAttributeString(const tinyxml2::XMLElement* elem, const std::string& name)
	{
		const char* buffer;

		xml::XMLError result = elem->QueryStringAttribute(name.c_str(), &buffer);
		if (FAILED(result))
			return "";

		std::string returnStr(buffer);
		return returnStr;
	}

	float GetSafeAttributeFloat(const tinyxml2::XMLElement* elem, const std::string& name)
	{
		float returnVal = 0.0f;

		xml::XMLError result = elem->QueryFloatAttribute(name.c_str(), &returnVal);

		return returnVal;
	}

	unsigned int GetSafeAttributeUint(const tinyxml2::XMLElement* elem, const std::string& name)
	{
		unsigned int returnVal = 0;

		xml::XMLError result = elem->QueryUnsignedAttribute(name.c_str(), &returnVal);

		return returnVal;
	}

	bool GetSafeAttributeBool(const tinyxml2::XMLElement* elem, const std::string& name)
	{
		bool returnVal = false;

		xml::XMLError result = elem->QueryBoolAttribute(name.c_str(), &returnVal);

		return returnVal;
	}
}