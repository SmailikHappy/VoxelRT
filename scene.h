#pragma once

#include "light.h"

// high level settings
#define WORLDSIZE 128 // power of 2. Warning: max 512 for a 512x512x512x4 bytes = 512MB world!

// low-level / derived
#define WORLDSIZE2	(WORLDSIZE*WORLDSIZE)
#define GRIDSIZE	WORLDSIZE
#define GRIDSIZE2	(GRIDSIZE*GRIDSIZE)
#define GRIDSIZE3	(GRIDSIZE*GRIDSIZE*GRIDSIZE)

#define MAXLIGHTS	32

// epsilon
#define EPSILON		0.00001f

namespace Tmpl8 {

class Scene
{
public:
	struct DDAState
	{
		int3 step;
		uint X, Y, Z;
		float t;
		float3 tdelta;
		float3 tmax;
	};

	struct SceneData
	{
		unsigned int grid[GRIDSIZE3]; // voxel payload is 'unsigned int', interpretation of the bits is free!
		Light lights[MAXLIGHTS];
		unsigned int lightCount;
	};

	Scene();

	void LoadDefaultLevel();
	bool LoadLevelFromFile(const char* filepath);
	bool SaveLevelToFile(const char* filepath);

	void FindNearest( Ray& ray ) const;
	bool IsOccluded( Ray& ray ) const;
	void Set( const uint x, const uint y, const uint z, const uint v );

	// RT funstions
	float3 ShadowRay(Light const& light, float3 const& pixelWorldPos, float3 const& pixelNormal) const;

	// Managment functions
	bool AddLight(const Light& light);
	bool RemoveLight(const int id);

	vector<Light>& GetLights() { return lights; }

	vector<Light> lights;
	uint *grid;

private:
	bool Setup3DDDA( Ray& ray, DDAState& state ) const;
};

}