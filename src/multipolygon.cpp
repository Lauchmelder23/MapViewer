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

struct Ring {
	std::vector<osmp::Relation::Member> members;
	int ring;
};

// Map values from one interval [A, B] to another [a, b]
inline double Map(double A, double B, double a, double b, double x)
{
	return (x - A) * (b - a) / (B - A) + a;
}

// TODO: Implement better algorithm
bool SelfIntersecting(const Ring & ring)
{
	struct Segment {
		std::shared_ptr<osmp::Node> p1, p2;
	};
	
	// Get all segments
	std::vector<Segment> segments;
	for (const osmp::Relation::Member member : ring.members)
	{
		std::vector<std::shared_ptr<osmp::Node>> nodes = std::dynamic_pointer_cast<osmp::Way>(member.member)->GetNodes();
		for (auto it = nodes.begin(); it != nodes.end() - 1; it++)
		{
			segments.push_back({
				*it, *(it + 1)
			});
		}
	}

	// Check for self intersection (O(n^2)...)
	for (auto it = segments.begin(); it != segments.end(); it++)
	{
		for (auto jt = segments.begin(); jt != segments.end(); jt++)
		{
			if (it == jt) continue;

			double A1 = (it->p1->lat - it->p2->lat) / (it->p1->lon - it->p2->lon);
			double A2 = (jt->p1->lat - jt->p2->lat) / (jt->p1->lon - jt->p2->lon);
			if (A1 == A2)	// Parallel
				continue;

			double b1 = it->p1->lat - A1 * it->p1->lon;
			double b2 = jt->p1->lat - A2 * jt->p1->lon;
			
			double Xa = (b2 - b1) / (A1 - A2);
			double Ya = A1 * Xa + b1;

			if ((Xa < std::max(std::min(it->p1->lon, it->p2->lon), std::min(jt->p1->lon, jt->p2->lon))) ||
				(Xa > std::min(std::max(it->p1->lon, it->p2->lon), std::max(jt->p1->lon, jt->p2->lon))))
				continue;
			else
				return true;
		}
	}

	return false;
}

Multipolygon::Multipolygon(const std::shared_ptr<osmp::Relation>& relation, int width, int height, osmp::Bounds bounds) :
	r(255), g(0), b(255), visible(true), rendering(RenderType::FILL), id(relation->id)
{
	if (relation->HasNullMembers())
		return;

	const std::vector<osmp::Relation::Member>& members = relation->GetWays();

	// Implement https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
	// Ring assignment

	// RA-1
	std::vector<Ring> rings;
	int ringCount = 0;

	// RA-2
	rings.push_back({ {members[0]}, ringCount });

	// RA-3
	if (rings[ringCount].members.front().member == rings[ringCount].members.back().member)
	{
		if (SelfIntersecting(rings[ringCount]))
		{
			// Backtracking ??
		}
	}
	else // RA-4
	{

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
