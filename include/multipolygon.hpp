#pragma once

#include <memory>

#include <osmrelation.hpp>

struct SDL_FPoint;
struct SDL_Renderer;

class Multipolygon
{
public:
	Multipolygon(const std::shared_ptr<osmp::Relation>& relation, int width, int height, osmp::Bounds bounds);

	void SetColor(int r, int g, int b);
	void Draw(SDL_Renderer* renderer);

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
	bool visible;
	enum RenderType {
		FILL,
		OUTLINE,
		INDOOR
	} rendering;
};