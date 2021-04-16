#pragma once

#include <string>
#include <vector>
#include <map>

#include <util.hpp>
#include <osmtag.hpp>

namespace osmp
{
	class Object;

	class IMember 
	{
	public:
		enum class Type {
			NODE, WAY, RELATION
		};

	public:
		IMember(const IMember& other) = delete;
		virtual ~IMember() {}

		IMember::Type GetType() const;

		const std::vector<Tag>& GetTags() const;
		size_t GetTagsSize() const;
		const Tag& GetTag(size_t index) const;
		std::string GetTag(const std::string& key) const;

	protected:
		IMember(const tinyxml2::XMLElement* element, Object* parent, IMember::Type type);

	protected:
		IMember::Type type;
		Object* parent;

		std::vector<Tag> tags;
		// std::map<std::string, std::string> tags;
		
	public:
		unsigned int id;
		std::string user;
		unsigned int uid;
		bool visible;
		std::string version;
		unsigned int changeset;
		std::string timestamp;
	};
}