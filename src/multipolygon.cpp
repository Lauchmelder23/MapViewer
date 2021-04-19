#include "..\include\multipolygon.hpp"

#include <vector>
#include <array>
#include <algorithm>
#include <map>
#include <iostream>

#include <triangle.h>
#include <osmway.hpp>
#include <osmnode.hpp>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>

#define BREAKIF(x) if(relation->id == x) __debugbreak()
#define INDEXOF(x, y, n) (y * n + x)

struct TriangulationData {
	std::vector<REAL> vertices, holes;
	std::vector<int> segments;
};

struct Ring {
	osmp::Nodes nodes;
	bool inner;
	int index;
	bool hole = false;
};

struct RingGroup {
	std::vector<Ring> rings;
};

// Map values from one interval [A, B] to another [a, b]
inline double Map(double A, double B, double a, double b, double x)
{
	return (x - A) * (b - a) / (B - A) + a;
}

// TODO: Implement better algorithm
[[nodiscard]] bool Intersect(double p1_x, double p1_y, double p2_x, double p2_y, double q1_x, double q1_y, double q2_x, double q2_y);
[[nodiscard]] bool Intersect(const osmp::Node& p1, const osmp::Node& p2, const osmp::Node& q1, const osmp::Node& q2);
[[nodiscard]] bool SelfIntersecting(const Ring& ring);

[[nodiscard]] bool BuildRing(Ring& ring, osmp::MemberWays& unassigned, int ringCount);
[[nodiscard]] bool AssignRings(std::vector<Ring>& rings, const osmp::MemberWays& members);

void FindAllContainedRings(const std::vector<bool>& containmentMatrix, int container, int numRings, std::vector<int>& buffer);
void FindAllContainedRingsThatArentContainedByUnusedRings(const std::vector<bool>& containmentMatrix, int container, int numRings, const std::vector<Ring>& unusedRings, std::vector<int>& buffer);
[[nodiscard]] int FindUncontainedRing(const std::vector<bool>& containmentMatrix, int rings, const std::vector<Ring>& unusedRings);
[[nodiscard]] bool PointInsideRing(const Ring& ring, const osmp::Node& point);
[[nodiscard]] bool IsRingContained(const Ring& r1, const Ring& r2);
[[nodiscard]] bool GroupRings(std::vector<RingGroup>& ringGroup, std::vector<Ring>& rings);

Multipolygon::Multipolygon(const osmp::Relation& relation, int width, int height, const osmp::Bounds& bounds) :
	r(255), g(0), b(255), visible(true), rendering(RenderType::FILL), id(relation->id)
{
	if (relation->HasNullMembers())
		return;

	const osmp::MemberWays& members = relation->GetWays();

	/* Implement https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm */

	std::vector<Ring> rings;
	if (!AssignRings(rings, members))
	{
		std::cerr << "Assigning rings has failed for multipolygon " << id << std::endl;
	}

	std::vector<RingGroup> ringGroups;
	GroupRings(ringGroups, rings);

	char* triSwitches = "zpNBQ";
	for (const RingGroup& ringGroup : ringGroups) 
	{
		TriangulationData td;

		bool valid = true;
		for (const Ring& ring : ringGroup.rings)
		{
			std::vector<REAL> vertices;
			for (const osmp::Node& node : ring.nodes) {
				double x = Map(bounds.minlon, bounds.maxlon, 0, width, node->lon);
				double y = height - Map(bounds.minlat, bounds.maxlat, 0, height, node->lat);

				vertices.push_back(x);
				vertices.push_back(y);
			}

			int segment = td.vertices.size() / 2;
			for (int i = 0; i < vertices.size() / 2; i += 1) {
				td.segments.push_back(segment + i);
				td.segments.push_back(segment + i + 1);
			}
			td.segments.back() = td.vertices.size() / 2;

			td.vertices.insert(td.vertices.end(), vertices.begin(), vertices.end());

			if (ring.hole) {
				double holeX = 0.0f;
				double holeY = 0.0f;
				for (int i = 0; i < vertices.size(); i += 2)
				{
					holeX += vertices[i];
					holeY += vertices[i + 1];
				}

				holeX /= vertices.size() / 2;
				holeY /= vertices.size() / 2;

				td.holes.push_back(holeX);
				td.holes.push_back(holeY);
			}
		}

		// TODO: Find better way to check for duplicates
		for (int i = 0; i < td.vertices.size(); i += 2) {
			for (int j = 0; j < td.vertices.size(); j += 2) {
				if (i == j) continue;

				if (td.vertices[i] == td.vertices[j] && td.vertices[i + 1] == td.vertices[j + 1])
				{
					valid = false;
					break;
				}
			}
		}
		
		if (valid)
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

			triangulate(triSwitches, &in, &out, NULL);

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

	//for (const Polygon& polygon : polygons) {
	//	for (auto it = polygon.vertices.begin(); it != polygon.vertices.end() - 1; it++) {
	//		thickLineRGBA(renderer,
	//			it->x, it->y,
	//			(it+1)->x, (it+1)->y,
	//			2,
	//			r, g, b, 255
	//			);
	//	}
	//}

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

bool Intersect(double p0_x, double p0_y, double p1_x, double p1_y, double p2_x, double p2_y, double p3_x, double p3_y)
{
	if ((p0_x == p2_x && p0_y == p2_y) || 
		(p0_x == p3_x && p0_y == p3_y) ||
		(p1_x == p2_x && p1_y == p2_y) ||
		(p1_x == p3_x && p1_y == p3_y)) 
		return false;

	float s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

	float s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		// Collision detected
		return 1;
	}

	return 0; // No collision
}

bool Intersect(const osmp::Node& p0, const osmp::Node& p1, const osmp::Node& p2, const osmp::Node& p3)
{
	return Intersect(p0->lon, p0->lat, p1->lon, p1->lat, p2->lon, p2->lat, p3->lon, p3->lat);
}
bool SelfIntersecting(const Ring& ring)
{
	struct Segment {
		osmp::Node p1, p2;
	};

	// Get all segments
	std::vector<Segment> segments;
	for (auto it = ring.nodes.begin(); it != ring.nodes.end(); it++)
	{
		if (it == ring.nodes.end() - 1)
		{
			segments.push_back({
				*it, ring.nodes.front()
				});
		}
		else
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

			if (Intersect(it->p1, it->p2, jt->p1, jt->p2)) 
				return true;
		}
	}

	return false;
}

bool BuildRing(Ring& ring, osmp::MemberWays& unassigned, int ringCount)
{
	const osmp::MemberWays original = unassigned;

	// RA-2
	int attempts = 0;
	ring = Ring{ unassigned[attempts].way->GetNodes(), unassigned[attempts].role == "inner", ringCount };
	unassigned.erase(unassigned.begin() + attempts);

RA3:
	// RA-3
	if (ring.nodes.front() == ring.nodes.back())
	{
		if (SelfIntersecting(ring))
		{
			unassigned = original;
			attempts += 1;
			if (unassigned.size() == attempts)
				return false;

			ring = Ring{ unassigned[attempts].way->GetNodes(), unassigned[attempts].role == "inner", ringCount };
			goto RA3;
		}
		else
		{
			ring.nodes.pop_back();
			return true;
		}
	}
	else // RA-4
	{
		osmp::Node lastNode = ring.nodes.back();
		for (auto it = unassigned.begin(); it != unassigned.end(); it++)
		{
			if (it->way->GetNodes().front() == lastNode)
			{
				ring.nodes.insert(ring.nodes.end(), it->way->GetNodes().begin() + 1, it->way->GetNodes().end());
				unassigned.erase(it);
				goto RA3;
			}
			else if (it->way->GetNodes().back() == lastNode)
			{
				ring.nodes.insert(ring.nodes.end(), it->way->GetNodes().rbegin() + 1, it->way->GetNodes().rend());
				unassigned.erase(it);
				goto RA3;
			}
		}

		// No ring found
		unassigned = original;
		return false;
	}
}

bool AssignRings(std::vector<Ring>& rings, const osmp::MemberWays& members)
{
	// Ring assignment
	osmp::MemberWays unassigned = members;
	int ringCount = 0;
	while (!unassigned.empty())
	{
		rings.push_back({});
		if (!BuildRing(rings.back(), unassigned, ringCount) || rings.size() > members.size())
			return false;

		ringCount++;
	}

	return true;
}

bool cmp(const osmp::Node& a, const osmp::Node& b) 
{
	return (a->lon < b->lon);
}

void FindAllContainedRings(const std::vector<bool>& containmentMatrix, int container, int numRings, std::vector<int>& buffer)
{
	buffer.clear();
	for (int j = 0; j < numRings; j++) {
		if (containmentMatrix[INDEXOF(container, j, numRings)])
			buffer.push_back(j);
	}
}

void FindAllContainedRingsThatArentContainedByUnusedRings(const std::vector<bool>& containmentMatrix, int container, int numRings, const std::vector<Ring>& unusedRings, std::vector<int>& buffer)
{
	FindAllContainedRings(containmentMatrix, container, numRings, buffer);

	for (auto j = buffer.begin(); j != buffer.end(); ) {
		bool foundRing = false;
		for (const Ring& ring : unusedRings) {
			if (containmentMatrix[INDEXOF(ring.index, *j, numRings)]) {
				foundRing = true;
				break;
			}
		}

		if(foundRing) 
			j = buffer.erase(j);
		else
			++j;
	}
}

bool Compare(const Ring& ring, int index) {
	return (ring.index == index);
}

int FindUncontainedRing(const std::vector<bool>& containmentMatrix, int rings, const std::vector<Ring>& unusedRings)
{
	for (int j = 0; j < rings; j++) {
		if (std::find_if(unusedRings.begin(), unusedRings.end(), [j](const Ring& ring) { return (ring.index == j); }) == unusedRings.end())
			continue;

		bool isContained = false;
		for (int i = 0; i < rings; i++) {
			if (containmentMatrix[INDEXOF(i, j, rings)])
			{
				isContained = true;
				break;
			}
		}

		if (!isContained)
			return j;
	}

	return -1;
}

bool PointInsideRing(const Ring& ring, const osmp::Node& point)
{
	const osmp::Node& rightestNode = *(std::max_element(ring.nodes.begin(), ring.nodes.end(), cmp));
	
	int intersections = 0;
	for (auto it = ring.nodes.begin(); it != ring.nodes.end(); it++)
	{
		const osmp::Node& jt = ((it == ring.nodes.end() - 1) ? ring.nodes.front() : *(it + 1));
		intersections += Intersect((*it)->GetLon(), (*it)->GetLat(),
			jt->GetLon(), jt->GetLat(), 
			point->GetLon(), point->GetLat(), 
			rightestNode->GetLon() + 1.0f, point->GetLat()
		);
	}

	return (intersections % 2 != 0);
}

bool IsRingContained(const Ring& r1, const Ring& r2)
{
	// Test if any line segments are intersecting
	// I don't think this is needed actually, the rings shouldn't overlap so testing if a node is inside is enough!
	// Gonna leave this here tho in case we *do* need to see if a ring is completely contained
	//for (auto it = r1.nodes.begin(); it != r1.nodes.end(); it++)
	//{
	//	for (auto jt = r2.nodes.begin(); jt != r2.nodes.end(); jt++)
	//	{
	//		osmp::Node n1 = ((it == r1.nodes.end() - 1) ? r1.nodes.front() : *(it + 1));
	//		osmp::Node n2 = ((jt == r2.nodes.end() - 1) ? r2.nodes.front() : *(jt + 1));

	//		if (Intersect(*it, n1, *jt, n2))
	//			return false;
	//	}
	//}

	if (PointInsideRing(r1, r2.nodes.front()))
		return true;

	return false;
}

bool GroupRings(std::vector<RingGroup>& ringGroups, std::vector<Ring>& rings)
{
	const std::vector<Ring> original = rings;

	//RG-1
	int ringNum = rings.size();
	std::vector<bool> containmentMatrix(ringNum * ringNum);

	for (int i = 0; i < ringNum; i++)
	{
		for (int j = 0; j < ringNum; j++)
		{
			if (i == j) {
				containmentMatrix[INDEXOF(i, j, ringNum)] = false;
				continue;
			}

			containmentMatrix[INDEXOF(i, j, ringNum)] = IsRingContained(rings[i], rings[j]);
		}
	}
	
	// RG-2 / RG-3
	while (!rings.empty())	// TODO: Make this time out
	{
		int uncontainedRing = FindUncontainedRing(containmentMatrix, ringNum, rings);
		if (uncontainedRing == -1) {
			std::cerr << "Failed to find uncontained ring in step RG-2" << std::endl;
			return false;
		}
		auto it = std::find_if(rings.begin(), rings.end(), [uncontainedRing](const Ring& ring) { return (ring.index == uncontainedRing); });
		if (it == rings.end())
		{
			std::cerr << "Uncontained Ring is out of range" << std::endl;
			return false;
		}

		ringGroups.push_back({ {*it} });
		rings.erase(it);


		// RG-4
		std::vector<int> containedRings;
		FindAllContainedRingsThatArentContainedByUnusedRings(containmentMatrix, uncontainedRing, ringNum, rings, containedRings);

		for (auto it = rings.begin(); it != rings.end(); ) {
			if (std::find(containedRings.begin(), containedRings.end(), it->index) != containedRings.end()) {
				ringGroups.back().rings.push_back(*it);
				ringGroups.back().rings.back().hole = true;
				it = rings.erase(it);
			}
			else {
				it++;
			}
		}

		// TODO: RG-5 / RG-6 will be left out for now as they're optional. 
		// At least RG-6 should be implemented because not doing so might crash Triangle

	}

	return true;
}
