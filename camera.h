#pragma once

// default screen resolution


#ifdef _DEBUG

#define SCRWIDTH	320
#define SCRHEIGHT	200

#else

#define SCRWIDTH	640
#define SCRHEIGHT	400

#endif 


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

	const float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	float3 camPos, camAhead;
	float3 topLeft, topRight, bottomLeft;
	float sensitivity = 0.01f;
	float speed = 0.015f;
};

}