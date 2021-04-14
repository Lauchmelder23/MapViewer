#include <iostream>
#include <vector>

#include <osmp.hpp>
#include <SDL.h>

// Map values from one interval [A, B] to another [a, b]
inline float Map(float A, float B, float a, float b, float x);

// A structure to hold a sequence of 2D points
typedef struct sRenderableWay
{
	size_t length;
	SDL_FPoint* points;
} RenderableWay;

int main(int argc, char** argv)
{
	// Load map data and calculate window size
	osmp::Object* obj = new osmp::Object("bigmap.osm");
	osmp::Bounds bounds = obj->bounds;
	float aspectRatio = (float)(bounds.maxlon - bounds.minlon) / (float)(bounds.maxlat - bounds.minlat);
	int windowWidth = 800;
	int windowHeight = windowWidth / aspectRatio;

	// Fetch all the ways
	std::vector<osmp::Way*> ways = obj->GetWays();

	// Turn them into renderable ways by mapping the global coordinates to screen coordinates (do this smarter in the future pls)
	std::vector<RenderableWay> rWays;
	for (osmp::Way* way : ways)
	{
		std::vector<const osmp::Node*> nodes = way->GetNodes();

		RenderableWay rWay;
		rWay.length = nodes.size();
		rWay.points = new SDL_FPoint[rWay.length];

		for (int i = 0; i < rWay.length; i++)
		{
			rWay.points[i].x = Map(bounds.minlon, bounds.maxlon, 0, windowWidth, nodes[i]->lon);
			rWay.points[i].y = 1000 - Map(bounds.minlat, bounds.maxlat, 0, windowHeight, nodes[i]->lat);
		}

		rWays.push_back(rWay);
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

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Render the ways
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		for (RenderableWay rWay : rWays)
		{
			SDL_RenderDrawLinesF(renderer, rWay.points, rWay.length);
		}

		SDL_RenderPresent(renderer);
	}

	// Cleanup time
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	for (RenderableWay& rWay : rWays)
		delete[] rWay.points;

	return 0;
}

inline float Map(float A, float B, float a, float b, float x)
{
	return (x - A) * (b - a) / (B - A) + a;
}
