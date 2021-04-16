#include "..\include\multipolygon.hpp"

#include <vector>

#include <triangle.h>
#include <osmway.hpp>
#include <osmnode.hpp>

struct TriangulationData {
	std::vector<REAL> vertices, holes;
};

// Map values from one interval [A, B] to another [a, b]
inline float Map(float A, float B, float a, float b, float x)
{
	return (x - A) * (b - a) / (B - A) + a;
}

Multipolygon::Multipolygon(const std::shared_ptr<osmp::Relation>& relation, int width, int height, osmp::Bounds bounds)
{
	if (relation->HasNullMembers())
		return;

	std::vector<TriangulationData> data;

	const std::vector<osmp::Relation::Member>& ways = relation->GetWays();
	std::vector<std::shared_ptr<osmp::Node>> nodes;
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
		const std::vector<std::shared_ptr<osmp::Node>> wayNodes = way->GetNodes();
		nodes.insert(nodes.end(), wayNodes.begin(), wayNodes.end());

		if (!(way->closed)) {
			if (nodes.front() == nodes.back()) 
			{
				nodes.pop_back();
			}
			else 
			{
				continue;
			}
		}

		if (!inner)
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

		td.vertices.insert(td.vertices.end(), vertices.begin(), vertices.end());
		nodes.clear();
	}

	char* triswitches = "zp";
	for (TriangulationData& td : data)
	{
		triangulateio in;

		in.numberofpoints = td.vertices.size() / 2;
		in.pointlist = td.vertices.data();
		
		in.numberofpointattributes = 0;
		in.numberofpointattributes = NULL;

		in.numberofholes = td.vertices.size() / 2;
		in.holelist = td.holes.data();

		in.numberofregions = 0;
		in.regionlist = NULL;

		triangulateio out;
		triangulate(triswitches, &in, &out, NULL);

		volatile int lol = 3;
	}
}
