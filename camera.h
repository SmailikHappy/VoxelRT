#pragma once

// default screen resolution


// #define FULLSCREEN
#define DOUBLESIZE

namespace Tmpl8 {

class Camera
{
public:
	Camera();
	~Camera();
	Ray GetPrimaryRay(const float x, const float y);
	bool HandleInput(const float dt, const int2& mouseMovement);

	const float aspect = (float)RENDERWIDTH / (float)RENDERHEIGHT;
	float3 camPos, camAhead;
	float3 topLeft, topRight, bottomLeft;
	float sensitivity = 0.01f;
	float speed = 0.015f;
};

}