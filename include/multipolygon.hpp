#pragma once

#include <memory>

#include <osmrelation.hpp>

class Multipolygon
{
public:
	Multipolygon(const std::shared_ptr<osmp::Relation>& relation, int width, int height, osmp::Bounds bounds);

private:
	
};