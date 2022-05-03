#pragma once

#include <memory>

#include <osmp.hpp>

class Multipolygon
{
public:
	Multipolygon(const osmp::Relation& relation, int width, int height, const osmp::Bounds& bounds);

	void SetColor(int r, int g, int b);
	void Draw();

	bool operator < (const Multipolygon& other) const {
		return (rendering < other.rendering);
	}

private:
	struct Vertex {
		double x, y;
	};
	struct Polygon {
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		std::vector<int> segments;
	};

	std::vector<Polygon> polygons;
	int r;
	int g;
	int b;
	uint64_t id;
	bool visible;
	enum RenderType {
		FILL,
		OUTLINE,
		INDOOR
	} rendering;
};