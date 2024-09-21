#pragma once

// default screen resolution
#define SCRWIDTH	640
#define SCRHEIGHT	400
// #define FULLSCREEN
#define DOUBLESIZE

namespace Tmpl8 {

class Camera
{
public:
	Camera();
	~Camera();
	Ray GetPrimaryRay(const float x, const float y);
	bool HandleInput(const float dt, const int2 const& mouseMovement);

	const float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	float3 camPos, camAhead;
	float3 topLeft, topRight, bottomLeft;
	const float sensitivity = 0.01f;
	const float speed = 0.0015f;
};

}