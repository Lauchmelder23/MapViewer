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

		[[nodiscard]] IMember::Type GetType() const;

		[[nodiscard]] const TagList& GetTags() const;
		[[nodiscard]] size_t GetTagsSize() const;
		[[nodiscard]] const Tag& GetTag(size_t index) const;
		[[nodiscard]] std::string GetTag(const std::string& key) const;

	protected:
		IMember(const tinyxml2::XMLElement* element, Object* parent, IMember::Type type);

	protected:
		IMember::Type type;
		Object* parent;

		TagList tags;
		
	public:
		uint64_t id;
		std::string user;
		unsigned int uid;
		bool visible;
		std::string version;
		unsigned int changeset;
		std::string timestamp;
	};
}