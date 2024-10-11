#pragma once

#define MAXRAYSTEPS 2

#include <vector>
#include <array>
#include <memory>
#include "light.h"

namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	void Tick( float deltaTime );
	void UI();
	void Shutdown() { /* nothing here for now */ }
	// input handling
	void MouseUp( int button ) { button = 0; /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { button = 0; /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y )
	{
	#if defined(DOUBLESIZE) && !defined(FULLSCREEN)
		int2 newPos{ x / 2, y / 2 };
	#else
		int2 newPos{ x, y };
	#endif

		dMousePos = newPos - mousePos;
		mousePos = newPos;
	}
	void MouseWheel( float y ) { y = 0; /* implement if you want to handle the mouse wheel */ }
	void KeyUp( int key ) { key = 0; /* implement if you want to handle keys */ }
	void KeyDown( int key ) { key = 0; /* implement if you want to handle keys */ }
	// data members
	int2 mousePos;
	int2 dMousePos;
	Scene scene;
	Camera camera;

	int selectedLightIndex = -1;

	char levelFilepath[256] = "C:\\Projects\\VoxelRT\\level.bin";
	bool accumulationEnabled = true;

	float frameTime, fps;
	uint rayCount;
	float imageAccumulationIndex;

	std::array<uint, RENDERHEIGHT> verticalIter;
	std::array<uint, RENDERWIDTH> horizontalIter;

	// RT functions
	float3 Trace(Ray& ray, int rayStep);
	float3 GetSkyColor(Ray& ray);
};

} // namespace Tmpl8