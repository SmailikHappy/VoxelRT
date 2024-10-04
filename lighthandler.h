#include <memory>
#include "light.h"

namespace Tmpl8
{
	class Scene;
}

#pragma once
class LightHandler
{
public:
	// RT funstions
	float3 ShadowRay(unique_ptr<Light> const& light, float3 const& pixelWorldPos, float3 const& pixelNormal) const;

	// Managment functions
	bool AddLight(const Light& light);
	bool RemoveLight(vector<unique_ptr<Light>>::iterator it);
	bool RemoveLight(const int id);

	const vector<unique_ptr<Light>>& GetLights() const { return lights; }

private:
	friend class Tmpl8::Scene;
	// Only Scene class is allowed to create light handlers

	LightHandler(Tmpl8::Scene* scene_) : scene(scene_) {}

	Tmpl8::Scene* scene;
	vector<unique_ptr<Light>> lights;
};

