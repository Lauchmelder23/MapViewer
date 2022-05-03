#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include <osmp.hpp>
#include "multipolygon.hpp"
#include "Window.hpp"

// Map values from one interval [A, B] to another [a, b]
inline float Map(float A, float B, float a, float b, float x);

typedef struct sArea
{
	size_t   length;
	uint8_t  r = 0;
	uint8_t  g = 0;
	uint8_t  b = 10;
	int16_t* x;
	int16_t* y;
} Area;

typedef struct sHighway
{
	size_t length;
	uint8_t r, g, b;
	Vector2f* points;
} Highway;

int main(int argc, char** argv)
{
	Window::Init();

	std::cout << "Loading and parsing OSM XML file. This might take a bit..." << std::flush;
	osmp::Object* obj = new osmp::Object("leipzig.osm");
	std::cout << "Done!" << std::endl;
	osmp::Bounds bounds = obj->bounds;
	float aspectRatio = (float)(bounds.maxlon - bounds.minlon) / (float)(bounds.maxlat - bounds.minlat);
	int windowHeight = 900 - 100;
	int windowWidth = windowHeight * aspectRatio;

	// Fetch all the ways
	osmp::Ways ways = obj->GetWays();

	// Turn them into renderable ways by mapping the global coordinates to screen coordinates (do this smarter in the future pls)
	std::vector<Area> buildings;
	std::vector<Highway> highways;
	for (osmp::Way way : ways)
	{
		const osmp::Nodes& nodes = way->GetNodes();
		std::string highwayVal = way->GetTag("highway");
		std::string railwayVal = way->GetTag("railway");
		if (way->area)
		{
			if (way->GetTag("building") == "")
			{
				continue;
			}

			Area area;
			area.length = nodes.size();
			area.x = new int16_t[area.length];
			area.y = new int16_t[area.length];

			area.r = 150;
			area.g = 150;
			area.b = 150;

			for (int i = 0; i < area.length; i++)
			{
				area.x[i] = Map(bounds.minlon, bounds.maxlon, 0, windowWidth, nodes[i]->lon);
				area.y[i] = windowHeight - Map(bounds.minlat, bounds.maxlat, 0, windowHeight, nodes[i]->lat);
			}

			buildings.push_back(area);
		}
		else if (highwayVal != "")
		{
			Highway highway;
			highway.length = nodes.size();
			highway.points = new Vector2f[highway.length];

			for (int i = 0; i < highway.length; i++)
			{
				highway.points[i].x = Map(bounds.minlon, bounds.maxlon, 0, windowWidth, nodes[i]->lon);
				highway.points[i].y = windowHeight - Map(bounds.minlat, bounds.maxlat, 0, windowHeight, nodes[i]->lat);
			}

			if (highwayVal == "motorway") { highway.r = 226; highway.g = 122; highway.b = 143; }
			else if (highwayVal == "trunk") { highway.r = 249; highway.g = 178; highway.b = 156; }
			else if (highwayVal == "primary") { highway.r = 252; highway.g = 206; highway.b = 144; }
			else if (highwayVal == "secondary") { highway.r = 244; highway.g = 251; highway.b = 173; }
			else if (highwayVal == "tertiary") { highway.r = 244; highway.g = 244; highway.b = 250; }
			else if (highwayVal == "footway") { highway.r = 233; highway.g = 140; highway.b = 124; }
			else { highway.r = 15; highway.g = 15; highway.b = 20; }

			highways.push_back(highway);
		}
		else if (railwayVal != "")
		{
			Highway railway;
			railway.length = nodes.size();
			railway.points = new Vector2f[railway.length];

			for (int i = 0; i < railway.length; i++)
			{
				railway.points[i].x = Map(bounds.minlon, bounds.maxlon, 0, windowWidth, nodes[i]->lon);
				railway.points[i].y = windowHeight - Map(bounds.minlat, bounds.maxlat, 0, windowHeight, nodes[i]->lat);
			}

			railway.r = 80; railway.g = 80; railway.b = 80;

			highways.push_back(railway);
		}
	}

	// Fetch all relations
	osmp::Relations relations = obj->GetRelations();
	std::vector<Multipolygon> multipolygons;
	for (const osmp::Relation& relation : relations)
	{
		if (relation->GetRelationType() == "multipolygon" && !relation->HasNullMembers())
		{
			Multipolygon mp = Multipolygon(relation, windowWidth, windowHeight, obj->bounds);
			multipolygons.push_back(mp);
		}
	}

	std::sort(multipolygons.begin(), multipolygons.end());


	// Release map data
	relations.clear();
	ways.clear();
	delete obj;

	// Create Window + Renderer
	Window window(Vector2i{ 1280, 800 }, "Map Viewer");

	// Window loop
	while ((bool)window)
	{
		Window::PollEvents();

		window.Clear(0.2f, 0.0f, 0.2f, 1.0f);
		
		 for (Multipolygon& multipolygon : multipolygons) {
			// multipolygon.Draw(renderer);
		 }

		for (Area& area : buildings)
		{
			// filledPolygonRGBA(renderer, area.x, area.y, area.length, area.r, area.g, area.b, 255);
		}

		for (Highway& highway : highways)
		{
			// SDL_SetRenderDrawColor(renderer, highway.r, highway.g, highway.b, 255);
			// SDL_RenderDrawLinesF(renderer, highway.points, highway.length);
		}
		

		window.SwapBuffers();
	}

	// Cleanup time
	// SDL_DestroyRenderer(renderer);
	// SDL_DestroyWindow(window);

	// SDL_Quit();

	for (Area& area : buildings) {
		delete[] area.x;
		delete[] area.y;
	}

	for (Highway& highway : highways)
		delete[] highway.points;

	return 0;
}

inline float Map(float A, float B, float a, float b, float x)
{
	return (x - A) * (b - a) / (B - A) + a;
}
