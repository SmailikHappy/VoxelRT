#include "template.h"

Camera::Camera()
{
	// try to load a camera
	FILE* f = fopen( "camera.bin", "rb" );
	if (f)
	{
		fread( this, 1, sizeof( Camera ), f );
		fclose( f );
	}
	else
	{
		// setup a basic view frustum
		camPos = float3( 0, 0, -2 );
		camAhead = float3( 0, 0, 1 );
		topLeft = float3( -aspect, 1, 0 );
		topRight = float3( aspect, 1, 0 );
		bottomLeft = float3( -aspect, -1, 0 );
	}
}

Camera::~Camera()
{
	// save current camera
	FILE* f = fopen( "camera.bin", "wb" );
	fwrite( this, 1, sizeof( Camera ), f );
	fclose( f );
}

Ray Camera::GetPrimaryRay(const float x, const float y)
{
	// calculate pixel position on virtual screen plane
	const float u = (float)x * (1.0f / RENDERWIDTH);
	const float v = (float)y * (1.0f / RENDERHEIGHT);
	const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
	// return Ray( camPos, normalize( P - camPos ) );
	return Ray( camPos, P - camPos );
	// Note: no need to normalize primary rays in a pure voxel world
	// TODO: 
	// - if we have other primitives as well, we *do* need to normalize!
	// - there are far cooler camera models, e.g. try 'Panini projection'.
}

bool Camera::HandleInput(const float dt, const int2& mouseMovement)
{
	if (!WindowHasFocus()) return false;
	
	float dmove = speed * dt;
	
	float3 tmpUp( 0, 1, 0 );
	float3 right = normalize( cross( tmpUp, camAhead) );
	float3 up = normalize( cross(camAhead, right ) );
	
	bool changed = false;
	
	if (IsKeyDown(GLFW_KEY_LEFT))	camAhead = normalize(camAhead - sensitivity * dt * 0.1f * right), changed = true;
	if (IsKeyDown(GLFW_KEY_RIGHT))	camAhead = normalize(camAhead + sensitivity * dt * 0.1f * right), changed = true;
	if (IsKeyDown(GLFW_KEY_UP))		camAhead = normalize(camAhead + sensitivity * dt * 0.1f * up), changed = true;
	if (IsKeyDown(GLFW_KEY_DOWN))	camAhead = normalize(camAhead - sensitivity * dt * 0.1f * up), changed = true;

	// recalc after applied changes
	right = normalize(cross(tmpUp, camAhead));
	up = normalize(cross(camAhead, right));
	
	if (IsKeyDown( GLFW_KEY_A )) camPos -= dmove * right, changed = true;
	if (IsKeyDown( GLFW_KEY_D )) camPos += dmove * right, changed = true;
	if (GetAsyncKeyState( 'W' )) camPos += dmove * camAhead, changed = true;
	if (IsKeyDown( GLFW_KEY_S )) camPos -= dmove * camAhead, changed = true;
	if (IsKeyDown( GLFW_KEY_E )) camPos += dmove * tmpUp, changed = true;
	if (IsKeyDown( GLFW_KEY_Q )) camPos -= dmove * tmpUp, changed = true;
	
	
	topLeft = camPos + 2 * camAhead - aspect * right + up;
	topRight = camPos + 2 * camAhead + aspect * right + up;
	bottomLeft = camPos + 2 * camAhead - aspect * right - up;

	//camAhead = normalize(camAhead + sensitivity * mouseMovement.x * right);
	//camAhead = normalize(camAhead + sensitivity * mouseMovement.y * up);


	return changed;
}