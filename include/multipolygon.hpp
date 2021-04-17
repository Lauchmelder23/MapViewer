#pragma once

#include <memory>

#include <osmrelation.hpp>

struct SDL_FPoint;
struct SDL_Renderer;

class Multipolygon
{
public:
	Multipolygon(const std::shared_ptr<osmp::Relation>& relation, int width, int height, osmp::Bounds bounds);

	void Draw(SDL_Renderer* renderer, int r, int g, int b);

private:
	struct Vertex {
		double x, y;
	};
	struct Polygon {
		std::vector<Vertex> vertices;
		std::vector<int> indices;
	};

	std::vector<Polygon> polygons;
};