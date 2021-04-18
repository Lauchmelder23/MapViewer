#pragma once
#include <vector>
#include <memory>

#include <util.hpp>
#include <osmtag.hpp>
#include <osmimember.hpp>

namespace osmp
{
	class Object;

	class IWay : public IMember
	{
	public:
		IWay(const IWay& other) = delete;
		IWay(const IWay&& other) = delete;
		virtual ~IWay() {}

		friend Way CreateWay(const tinyxml2::XMLElement* way_elem, Object* parent);

		[[nodiscard]] const Nodes& GetNodes() const;
		[[nodiscard]] size_t GetNodesSize() const;
		[[nodiscard]] Node GetNode(size_t index) const;

	protected:
		IWay(const tinyxml2::XMLElement* way_elem, Object* parent);

	public:
		bool area, closed;	// Closed := Startpoint = endpoint, Area := Closed AND certain conditions are not met

	private:
		Nodes nodes;
	};
}