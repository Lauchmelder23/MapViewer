#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include <osmp.hpp>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <../include/multipolygon.hpp>

// Map values from one interval [A, B] to another [a, b]
inline float Map(float A, float B, float a, float b, float x);

typedef struct sArea
{
	size_t length;
	Uint8 r = 0;
	Uint8 g = 0;
	Uint8 b = 10;
	Sint16* x;
	Sint16* y;
} Area;

typedef struct sHighway
{
	size_t length;
	Uint8 r, g, b;
	SDL_FPoint* points;
} Highway;

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);
	// Load map data and calculate window size
	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);

	std::cout << "Loading and parsing OSM XML file. This might take a bit..." << std::flush;
	osmp::Object* obj = new osmp::Object("leipzig.osm");
	std::cout << "Done!" << std::endl;
	osmp::Bounds bounds = obj->bounds;
	float aspectRatio = (float)(bounds.maxlon - bounds.minlon) / (float)(bounds.maxlat - bounds.minlat);
	int windowHeight = DM.h - 100;
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
			area.x = new Sint16[area.length];
			area.y = new Sint16[area.length];

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
			highway.points = new SDL_FPoint[highway.length];

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
			railway.points = new SDL_FPoint[railway.length];

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
	SDL_Window* window = SDL_CreateWindow("MapViewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	if (window == nullptr)
	{
		std::cerr << "Failed to create Window" << std::endl;
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == nullptr)
	{
		std::cerr << "Failed to create renderer" << std::endl;
		return 1;
	}

	// Window loop
	bool isOpen = true;
	SDL_Event e;
	while (isOpen)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_WINDOWEVENT)
			{
				switch (e.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					isOpen = false;
					break;
				}
			}
		}

		SDL_SetRenderDrawColor(renderer, 240, 240, 250, 255);
		SDL_RenderClear(renderer);
		
		 for (Multipolygon& multipolygon : multipolygons) {
			multipolygon.Draw(renderer);
		 }

		for (Area& area : buildings)
		{
			filledPolygonRGBA(renderer, area.x, area.y, area.length, area.r, area.g, area.b, 255);
		}

		for (Highway& highway : highways)
		{
			SDL_SetRenderDrawColor(renderer, highway.r, highway.g, highway.b, 255);
			SDL_RenderDrawLinesF(renderer, highway.points, highway.length);
		}
		

		SDL_RenderPresent(renderer);
	}

	// Cleanup time
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

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
