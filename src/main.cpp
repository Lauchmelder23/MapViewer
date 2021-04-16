#include <iostream>
#include <vector>
#include <string>

#include <osmp.hpp>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <../include/multipolygon.hpp>

// Map values from one interval [A, B] to another [a, b]
inline float Map(float A, float B, float a, float b, float x);

// A structure to hold a sequence of 2D points
typedef struct sRenderableWay
{
	size_t length;
	SDL_FPoint* points;
} RenderableWay;

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
	// Load map data and calculate window size
	osmp::Object* obj = new osmp::Object("leipzig.osm");
	osmp::Bounds bounds = obj->bounds;
	float aspectRatio = (float)(bounds.maxlon - bounds.minlon) / (float)(bounds.maxlat - bounds.minlat);
	int windowWidth = 2000;
	int windowHeight = windowWidth / aspectRatio;

	// Fetch all the ways
	std::vector<std::shared_ptr<osmp::Way>> ways = obj->GetWays();

	// Turn them into renderable ways by mapping the global coordinates to screen coordinates (do this smarter in the future pls)
	std::vector<RenderableWay> rWays;
	std::vector<Area> areas;
	std::vector<Highway> highways;
	std::vector<Highway> railways;
	for (std::shared_ptr<osmp::Way> way : ways)
	{
		const std::vector<std::shared_ptr<osmp::Node>>& nodes = way->GetNodes();
		std::string highwayVal = way->GetTag("highway");
		std::string railwayVal = way->GetTag("railway");
		if (way->area)
		{
			Area area;
			area.length = nodes.size();
			area.x = new Sint16[area.length];
			area.y = new Sint16[area.length];

			for (int i = 0; i < area.length; i++)
			{
				area.x[i] = Map(bounds.minlon, bounds.maxlon, 0, windowWidth, nodes[i]->lon);
				area.y[i] = windowHeight - Map(bounds.minlat, bounds.maxlat, 0, windowHeight, nodes[i]->lat);
			}


			std::string tag = "";
			tag = way->GetTag("building");
			if (tag != "")
			{
				area.r = 100;
				area.g = 100;
				area.b = 100;
			}

			tag = way->GetTag("natural");
			if (tag != "")
			{
				if (tag == "wood" || tag == "scrub" || tag == "heath") {
					area.r = 47;
					area.g = 157;
					area.b = 0;
				}
				else if (tag == "water" || tag == "floodplain") {
					area.r = 106;
					area.g = 151;
					area.b = 255;
				}
				else if (tag == "grassland" || tag == "grass") {
					area.r = 143;
					area.g = 255;
					area.b = 106;
				}
				else if (tag == "sand") {
					area.r = 244;
					area.g = 255;
					area.b = 106;
				}
				else {
					std::cout << "Unknown natural: " << tag << std::endl;
				}
			}

			tag = way->GetTag("water");
			if (tag != "")
			{
				area.r = 106;
				area.g = 151;
				area.b = 255;
			}

			tag = way->GetTag("waterway");
			if (tag != "")
			{
				area.r = 106;
				area.g = 151;
				area.b = 255;
			}

			areas.push_back(area);
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
			else { highway.r = 244; highway.g = 244; highway.b = 250; }

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
		else
		{
			RenderableWay rWay;
			rWay.length = nodes.size();
			rWay.points = new SDL_FPoint[rWay.length];

			for (int i = 0; i < rWay.length; i++)
			{
				rWay.points[i].x = Map(bounds.minlon, bounds.maxlon, 0, windowWidth, nodes[i]->lon);
				rWay.points[i].y = windowHeight - Map(bounds.minlat, bounds.maxlat, 0, windowHeight, nodes[i]->lat);
			}

			rWays.push_back(rWay);
		}
	}

	// Fetch all relations
	std::vector<std::shared_ptr<osmp::Relation>> relations = obj->GetRelations();
	std::vector<Multipolygon> multipolygons;
	for (const std::shared_ptr<osmp::Relation>& relation : relations)
	{
		if (relation->GetRelationType() == "multipolygon" && !relation->HasNullMembers())
		{
			Multipolygon mp = Multipolygon(relation, windowWidth, windowHeight, obj->bounds);
			multipolygons.push_back(mp);
		}
	}


	// Release map data
	delete obj;

	// Initiaize graphics API
	SDL_Init(SDL_INIT_VIDEO);

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

		SDL_SetRenderDrawColor(renderer, 0, 0, 10, 255);
		SDL_RenderClear(renderer);

		// Render the ways
		/*
		SDL_SetRenderDrawColor(renderer, 255, 245, 245, 255);
		for (RenderableWay& rWay : rWays)
		{
			SDL_RenderDrawLinesF(renderer, rWay.points, rWay.length);
		}
		*/
		
		for (Area& area : areas)
		{
			filledPolygonRGBA(renderer, area.x, area.y, area.length, area.r, area.g, area.b, 255);
		}
		

		for (Highway& highway : highways)
		{
			SDL_SetRenderDrawColor(renderer, highway.r, highway.g, highway.b, 255);
			SDL_RenderDrawLinesF(renderer, highway.points, highway.length);
		}

		for (Highway& railway : railways)
		{
			SDL_SetRenderDrawColor(renderer, railway.r, railway.g, railway.b, 255);
			SDL_RenderDrawLinesF(renderer, railway.points, railway.length);
		}
		

		SDL_RenderPresent(renderer);
	}

	// Cleanup time
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	for (Area& area : areas) {
		delete[] area.x;
		delete[] area.y;
	}

	for (Highway& highway : highways)
		delete[] highway.points;

	for (RenderableWay& rWay : rWays)
		delete[] rWay.points;

	return 0;
}

inline float Map(float A, float B, float a, float b, float x)
{
	return (x - A) * (b - a) / (B - A) + a;
}
