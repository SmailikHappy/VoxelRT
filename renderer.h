#pragma once

namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	float3 Trace( Ray& ray, int = 0, int = 0, int = 0 );
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
	float3* accumulator;	// for episode 3
	float3* history;		// for episode 5
	Scene scene;
	Camera camera;
};

} // namespace Tmpl8