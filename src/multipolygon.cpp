#include "..\include\multipolygon.hpp"

#include <vector>
#include <algorithm>
#include <map>
#include <iostream>

#include <triangle.h>
#include <osmway.hpp>
#include <osmnode.hpp>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>

#define BREAKIF(x) if(relation->id == x) __debugbreak()

struct TriangulationData {
	std::vector<REAL> vertices, holes;
	std::vector<int> segments;
};

// Map values from one interval [A, B] to another [a, b]
inline double Map(double A, double B, double a, double b, double x)
{
	return (x - A) * (b - a) / (B - A) + a;
}

Multipolygon::Multipolygon(const std::shared_ptr<osmp::Relation>& relation, int width, int height, osmp::Bounds bounds) :
	r(255), g(0), b(255), visible(true), rendering(RenderType::FILL), id(relation->id)
{
	if (relation->HasNullMembers())
		return;

	// BREAKIF(7344428);
	// BREAKIF(6427823);

	std::vector<TriangulationData> data;

	std::vector<osmp::Relation::Member> ways = relation->GetWays();
	std::vector<std::shared_ptr<osmp::Node>> nodes;
	int run = 1;

	bool lastWasInner = false;
	bool hasSeenOuter = false;
	std::vector<std::vector<osmp::Relation::Member>> outerWays;
	std::vector<std::vector<osmp::Relation::Member>> innerWays;
	// Pre processing
	for (osmp::Relation::Member member : ways) {
		std::shared_ptr<osmp::Way> way = std::dynamic_pointer_cast<osmp::Way>(member.member);
		if (member.role == "inner") 
		{
			if (!hasSeenOuter)	// TODO: Find better way to sort things
				continue;

			if (innerWays.empty() || !lastWasInner)
				innerWays.push_back({});

			innerWays.back().push_back(member);
			lastWasInner = true;
		}
		else 
		{
			hasSeenOuter = true;
			if (outerWays.empty() || lastWasInner)
				outerWays.push_back({});

			outerWays.back().push_back(member);
			lastWasInner = false;
		}
	}

	if (outerWays.empty())	// There must always be an outer ring, anything else makes no sense
		return;

	auto jt = outerWays.begin();
	bool currentIsInner = false;
	while (!outerWays.empty() || !innerWays.empty())
	{
		std::vector<osmp::Relation::Member> member = *jt;
		auto it = member.begin();
		while (!member.empty())
		{
			if (it == member.end())
				it = member.begin();
			std::shared_ptr<osmp::Way> way = std::dynamic_pointer_cast<osmp::Way>(it->member);

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

			bool inner = (it->role == "inner");
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
				else if (nodes.front() == wayNodes.front()) {
					nodes.insert(nodes.begin(), wayNodes.rbegin(), wayNodes.rend() - 1);
				}
				else {
					it++;
					continue;
				}
			}

			it = member.erase(it);

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
			std::map<int, int> duplicates;
			int n = td.vertices.size() / 2;
			for (const std::shared_ptr<osmp::Node>& node : nodes) {
				double x = Map(bounds.minlon, bounds.maxlon, 0, width, node->lon);
				double y = height - Map(bounds.minlat, bounds.maxlat, 0, height, node->lat);

				auto xit = std::find(td.vertices.begin(), td.vertices.end(), x);
				auto yit = std::find(td.vertices.begin(), td.vertices.end(), y);
				if (std::distance(xit, yit) == 1) {
					duplicates.insert(std::make_pair(n, std::distance(td.vertices.begin(), xit) / 2));
				}
				else {
					vertices.push_back(x);
					vertices.push_back(y);
				}
				n++;
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
				auto dit = duplicates.find(segNum);
				if (dit != duplicates.end())
				{
					td.segments.push_back(dit->second);
				}
				else
				{
					td.segments.push_back(segNum++);
				}

				dit = duplicates.find(segNum);
				if (dit != duplicates.end())
				{
					td.segments.push_back(dit->second);
				}
				else
				{
					td.segments.push_back(segNum);
				}
			}
			td.segments.back() = td.vertices.size() / 2;

			td.vertices.insert(td.vertices.end(), vertices.begin(), vertices.end());
			nodes.clear();
			run = 1;
		}

		if (currentIsInner) {
			innerWays.erase(innerWays.begin());
			jt = outerWays.begin();
		}
		else {
			outerWays.erase(outerWays.begin());
			jt = innerWays.begin();
		}

		currentIsInner = !currentIsInner;
	}

	char* triswitches = "zpNBQ";
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
		for (int i = 0; i < in.numberofsegments * 2; i++) {
			polygons.back().segments.push_back(in.segmentlist[i]);
		}

		trifree(out.trianglelist);
		trifree(out.segmentlist);
	}


	// TODO: Make a color map

	std::string tag = "";
	tag = relation->GetTag("indoor");
	if (tag != "")
	{
		rendering = RenderType::INDOOR;
		r = 150;
		g = 150;
		b = 150;
	}

	tag = relation->GetTag("building");
	if (tag != "" || relation->GetTag("building:colour") != "" || relation->GetTag("building:material") != "" || relation->GetTag("building:part") != "")
	{
		r = 150;
		g = 150;
		b = 150;
	}

	tag = relation->GetTag("natural");
	if (tag != "")
	{
		if (tag == "wood") {
			r = 157;
			g = 202;
			b = 138;
		}
		else if (tag == "scrub") {
			r = 200;
			g = 215;
			b = 171;
		}
		else if (tag == "heath") {
			r = 214;
			g = 217;
			b = 159;
		}
		else if (tag == "water") {
			r = 166;
			g = 198;
			b = 198;
		}
		else if (tag == "grassland") {
			r = 205;
			g = 235;
			b = 176;
		}
		else if (tag == "floodplain") {
			r = 174;
			g = 236;
			b = 190;
		}
		else if (tag == "sand") {
			r = 234;
			g = 222;
			b = 189;
		}
		else if (tag == "scree") {
			r = 237;
			g = 228;
			b = 220;
		}
		else if (tag == "bare_rock") {
			r = 213;
			g = 209;
			b = 204;
		}
		else if (tag == "tree_row") {
			r = 169;
			g = 206;
			b = 161;
		}
	}

	tag = relation->GetTag("water");
	if (tag != "")
	{
		r = 106;
		g = 151;
		b = 255;
	}

	tag = relation->GetTag("waterway");
	if (tag != "")
	{
		r = 106;
		g = 151;
		b = 255;
	}

	tag = relation->GetTag("landuse");
	if (tag != "")
	{
		if (tag == "grass") 
		{
			r = 207;
			g = 237;
			b = 165;
		}
		else if (tag == "commercial")
		{
			r = 238;
			g = 205;
			b = 205;
		}
		else if (tag == "residential")
		{
			r = 218;
			g = 218;
			b = 218;
		}
		else if (tag == "forest")
		{
			r = 157;
			g = 202;
			b = 138;
		}
		else if (tag == "basin")
		{
			r = 170;
			g = 211;
			b = 223;
		}
		else if (tag == "allotments")
		{
			r = 201;
			g = 225;
			b = 191;
		}
		else if (tag == "railway")
		{
			r = 230;
			g = 209;
			b = 227;
		}
		else if (tag == "construction")
		{
			r = 199;
			g = 199;
			b = 180;
		}
		else if (tag == "retail")
		{
			r = 254;
			g = 202;
			b = 197;
		}
		else if (tag == "village_green")
		{
			r = 205;
			g = 235;
			b = 176;
		}
		else if (tag == "meadow")
		{
			r = 205;
			g = 236;
			b = 176;
		}
		else if (tag == "cemetery")
		{
			r = 170;
			g = 203;
			b = 175;
		}
		else if (tag == "brownfield") {
			r = 167;
			g = 168;
			b = 126;
		}
		else if (tag == "recreation_ground") {
			r = 223;
			g = 252;
			b = 226;
		}
	}

	tag = relation->GetTag("leisure");
	if (tag != "")
	{
		if (tag == "park")
		{
			r = 205;
			g = 247;
			b = 201;
		}
		else if (tag == "garden")
		{
			r = 205;
			g = 235;
			b = 176;
		}
		else if (tag == "pitch")
		{
			r = 170;
			g = 224;
			b = 203;
		}
		else if (tag == "sports_centre")
		{
			r = 223;
			g = 252;
			b = 226;
		}
		else if (tag == "track")
		{
			r = 170;
			g = 224;
			b = 203;
		}
		else if (tag == "slipway")
		{
			r = 0;
			g = 146;
			b = 218;
		}
		else if (tag == "playground")
		{
			r = 223;
			g = 252;
			b = 226;
		}
	}

	tag = relation->GetTag("tourism");
	if (tag != "")
	{
		if (tag == "zoo") {
			rendering = RenderType::OUTLINE;
			r = 147;
			g = 84;
			b = 115;
		}
	}

	tag = relation->GetTag("man_made");
	if (tag != "")
	{
		if (tag == "bridge") {
			r = 184;
			g = 184;
			b = 184;
		}
		else if (tag == "wastewater_plant") {
			r = 230;
			g = 209;
			b = 227;
		}
		else if (tag == "pier") {
			r = 250;
			g = 250;
			b = 255;
		}
	}

	tag = relation->GetTag("amenity");
	if (tag != "")
	{
		if (tag == "parking" || tag == "bicycle_parking") {
			r = 100;
			g = 100;
			b = 120;
		}
		else if (tag == "school" || tag == "university" || tag == "kindergarten") {
			r = 255;
			g = 255;
			b = 229;
		}
	}

	tag = relation->GetTag("place");
	if (tag != "")
	{
		r = 180;
		g = 180;
		b = 180;
	}

	tag = relation->GetTag("public_transport");
	if (tag != "")
	{
		if (tag == "platform")
		{
			r = 180;
			g = 180;
			b = 190;
		}
	}

	tag = relation->GetTag("highway");
	if (tag != "")
	{
		if (tag == "pedestrian")
		{
			r = 213;
			g = 212;
			b = 227;
		}
	}

	tag = relation->GetTag("area:highway");
	if (tag != "")
	{
		if (tag == "primary")
		{
			r = 255;
			g = 255;
			b = 229;
		}
		else if (tag == "secondary")
		{
			r = 244; 
			g = 251; 
			b = 173;
		}
		else if (tag == "footway" || tag == "cycleway" || tag == "footway;cycleway")	// TODO: Apparently you can list values??? check with the standard.
		{
			r = 233; 
			g = 140; 
			b = 124;
		}
		else if (tag == "emergency")
		{
			r = 250;
			g = 250;
			b = 255;
		}
		else if (tag == "unclassified" || tag == "emergency" || tag == "residential" || tag == "service" || tag == "traffic_island")
		{
			r = 15;
			g = 15;
			b = 20;
		}
		else if (tag == "bus") {
			r = 150;
			g = 150;
			b = 150;
		}
		else if (tag == "reserved") {	// TODO: Not a keyword I'm aware of
			r = 0; g = 0; b = 0;
			visible = false;
		}
	}

	tag = relation->GetTag("area:railway");
	if (tag != "")
	{
		if (tag == "tram") {
			r = 150;
			g = 150;
			b = 150;
		}
	}

	tag = relation->GetTag("bridge:support");
	if (tag != "")
	{
		r = 184;
		g = 184;
		b = 184;
	}

	tag = relation->GetTag("tunnel");
	if (tag == "yes")
	{
		r = 240;
		g = 240;
		b = 255;
	}

	if (r == 255 && b == 255) {
		std::cout << relation->id << std::endl;
	}
}

void Multipolygon::SetColor(int r, int g, int b)
{
	this->r = r;
	this->g = g;
	this->b = b;
}

void Multipolygon::Draw(SDL_Renderer* renderer)
{
	if (!visible)
		return;

	// if (id != 6427823)
	//	return;

	for (const Polygon& polygon : polygons) {
		switch(rendering)
		{
		case RenderType::FILL:
			for (int i = 0; i < polygon.indices.size(); i += 3)	// Be a graphics card
			{

				filledTrigonRGBA(renderer,
					polygon.vertices[polygon.indices[i + 0]].x, polygon.vertices[polygon.indices[i + 0]].y,
					polygon.vertices[polygon.indices[i + 1]].x, polygon.vertices[polygon.indices[i + 1]].y,
					polygon.vertices[polygon.indices[i + 2]].x, polygon.vertices[polygon.indices[i + 2]].y,
					r, g, b, 255
				);
			}
			break;

		case RenderType::OUTLINE:
			for(int i = 0; i < polygon.segments.size(); i += 2)
			{
				thickLineRGBA(renderer,
					polygon.vertices[polygon.segments[i + 0]].x, polygon.vertices[polygon.segments[i + 0]].y,
					polygon.vertices[polygon.segments[i + 1]].x, polygon.vertices[polygon.segments[i + 1]].y,
					5, r, g, b, 255
				);
			}
			break;

		case RenderType::INDOOR:
			for (int i = 0; i < polygon.indices.size(); i += 3)	// Be a graphics card
			{

				filledTrigonRGBA(renderer,
					polygon.vertices[polygon.indices[i + 0]].x, polygon.vertices[polygon.indices[i + 0]].y,
					polygon.vertices[polygon.indices[i + 1]].x, polygon.vertices[polygon.indices[i + 1]].y,
					polygon.vertices[polygon.indices[i + 2]].x, polygon.vertices[polygon.indices[i + 2]].y,
					r, g, b, 255
				);
			}

			for (int i = 0; i < polygon.segments.size(); i += 2)
			{
				lineRGBA(renderer,
					polygon.vertices[polygon.segments[i + 0]].x, polygon.vertices[polygon.segments[i + 0]].y,
					polygon.vertices[polygon.segments[i + 1]].x, polygon.vertices[polygon.segments[i + 1]].y,
					10, 10, 15, 255
				);
			}
			break;
		}
	}
}
