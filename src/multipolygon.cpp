#include "..\include\multipolygon.hpp"

#include <vector>

#include <triangle.h>
#include <osmway.hpp>
#include <osmnode.hpp>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>

struct TriangulationData {
	std::vector<REAL> vertices, holes;
	std::vector<int> segments;
};

// Map values from one interval [A, B] to another [a, b]
inline double Map(double A, double B, double a, double b, double x)
{
	return (x - A) * (b - a) / (B - A) + a;
}

Multipolygon::Multipolygon(const std::shared_ptr<osmp::Relation>& relation, int width, int height, osmp::Bounds bounds)
{
	if (relation->HasNullMembers())
		return;

	std::vector<TriangulationData> data;
	
	if (relation->id == 3363659) {
		__debugbreak();
	}

	const std::vector<osmp::Relation::Member>& ways = relation->GetWays();
	std::vector<std::shared_ptr<osmp::Node>> nodes;
	std::shared_ptr<osmp::Node> lastNode = nullptr;
	std::shared_ptr<osmp::Way> nextNode = nullptr;
	int run = 1;
	int total = 0;
	for (osmp::Relation::Member member : ways) 
	{
		std::shared_ptr<osmp::Way> way = std::dynamic_pointer_cast<osmp::Way>(member.member);

		// Several possible scenarios:
		// Closed way
		//		Outer edge
		//			Append all nodes to the triangulation data
		//		Inner edge
		//			Append all nodes to the triangulation data
		//			Calculate average of nodes to get coordinates of the hole
		//
		// Open way
		//		Read next way until way is closed. This MUST happen, if the way remains open the OSM data is faulty and should be discarded
		//		Continue with Closed way algorithm

		bool inner = (member.role == "inner");
		std::vector<std::shared_ptr<osmp::Node>> wayNodes = way->GetNodes();

		if (run == 1) {
			nodes.insert(nodes.begin(), wayNodes.begin(), wayNodes.end());
		}
		else {
			if (nodes.back() == wayNodes.front()) {
				nodes.insert(nodes.end(), wayNodes.begin() + 1, wayNodes.end());
			}
			else if (nodes.back() == wayNodes.back()) {
				nodes.insert(nodes.end(), wayNodes.rbegin() + 1, wayNodes.rend());
			}
			else if (nodes.front() == wayNodes.back()) {
				nodes.insert(nodes.begin(), wayNodes.begin(), wayNodes.end() - 1);
			}
			else /*if (nodes.front() == wayNodes.front())*/ {
				nodes.insert(nodes.begin(), wayNodes.rbegin(), wayNodes.rend() - 1);
			}
		}

		run++;

		if (!(way->closed)) {
			if (nodes.size() > 1 && nodes.front() == nodes.back()) 
			{
				// nodes.pop_back();
			}
			else 
			{
				continue;
			}
		}
		nodes.pop_back();

		if (!inner || data.empty())
		{
			data.push_back({});
		}
		TriangulationData& td = data.back();

		// Push all vertices to data
		std::vector<REAL> vertices;
		for (const std::shared_ptr<osmp::Node>& node : nodes) {
			vertices.push_back(Map(bounds.minlon, bounds.maxlon, 0, width, node->lon));
			vertices.push_back(height - Map(bounds.minlat, bounds.maxlat, 0, height, node->lat));
		}

		if (inner)
		{
			// Calculate data of hole by using the average position of all inner vertices (that should work right?, probably not...)
			REAL holeX = 0.0f;
			REAL holeY = 0.0f;;
			for (int i = 0; i < vertices.size(); i += 2)
			{
				holeX += vertices[i];
				holeY += vertices[i + 1];
			}
			holeX /= (vertices.size() / 2);
			holeY /= (vertices.size() / 2);

			td.holes.push_back(holeX);
			td.holes.push_back(holeY);
		}

		// Get segments
		int segNum = td.vertices.size() / 2;
		for (int i = 0; i < vertices.size(); i += 2) {
			td.segments.push_back(segNum);
			td.segments.push_back(++segNum);
		}
		td.segments.back() = td.vertices.size() / 2;

		td.vertices.insert(td.vertices.end(), vertices.begin(), vertices.end());
		nodes.clear();
		lastNode = nullptr;
		run = 1;
	}

	char* triswitches = "zpNBV";
	for (TriangulationData& td : data)
	{
		triangulateio in;

		in.numberofpoints = td.vertices.size() / 2;
		in.pointlist = td.vertices.data();
		in.pointmarkerlist = NULL;
		
		in.numberofpointattributes = 0;
		in.numberofpointattributes = NULL;

		in.numberofholes = td.holes.size() / 2;
		in.holelist = td.holes.data();

		in.numberofsegments = td.segments.size() / 2;
		in.segmentlist = td.segments.data();
		in.segmentmarkerlist = NULL;

		in.numberofregions = 0;
		in.regionlist = NULL;

		triangulateio out;
		out.pointlist = NULL;
		out.pointmarkerlist = NULL;
		out.trianglelist = NULL;
		out.segmentlist = NULL;
		out.segmentmarkerlist = NULL;
		triangulate(triswitches, &in, &out, NULL);

		// TODO: memory leak go brrrr
		polygons.push_back({});
		for (int i = 0; i < in.numberofpoints * 2; i += 2) {
			polygons.back().vertices.push_back({ in.pointlist[i], in.pointlist[i + 1] });
			// polygons.back().vertices.push_back(in.pointlist[i + 1]);
		}
		for (int i = 0; i < out.numberoftriangles * 3; i++) {
			polygons.back().indices.push_back(out.trianglelist[i]);
		}
	}
}

void Multipolygon::Draw(SDL_Renderer* renderer, int r, int g, int b)
{
	for (const Polygon& polygon : polygons) {
		for (int i = 0; i < polygon.indices.size(); i += 3)	// Be a graphics card
		{
			filledTrigonRGBA(renderer,
				polygon.vertices[polygon.indices[i + 0]].x, polygon.vertices[polygon.indices[i + 0]].y,
				polygon.vertices[polygon.indices[i + 1]].x, polygon.vertices[polygon.indices[i + 1]].y,
				polygon.vertices[polygon.indices[i + 2]].x, polygon.vertices[polygon.indices[i + 2]].y,
				r, g, b, 255
				);
		}
	}
}
