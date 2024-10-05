#pragma once

#include "light.h"
#include "material.h"
#include <map>

// high level settings
#define WORLDSIZE		128 // power of 2. Warning: max 512 for a 512x512x512x4 bytes = 512MB world!

// low-level / derived
#define WORLDSIZE2		(WORLDSIZE*WORLDSIZE)
#define GRIDSIZE		WORLDSIZE
#define GRIDSIZE2		(GRIDSIZE*GRIDSIZE)
#define GRIDSIZE3		(GRIDSIZE*GRIDSIZE*GRIDSIZE)

#define MAXLIGHTS		32
#define MAXMATERIALS	256

#define NOMATERIALKEY	0

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
		unsigned short grid[GRIDSIZE3]; // voxel payload is 'unsigned int', interpretation of the bits is free!
		Light lights[MAXLIGHTS];
		unsigned int lightCount;
		Material materials[MAXMATERIALS];
		unsigned int materialCount;
	};

	Scene();

	void LoadDefaultLevel();
	bool LoadLevelFromFile(const char* filepath);
	bool SaveLevelToFile(const char* filepath);

	void FindNearest( Ray& ray ) const;
	bool IsOccluded( Ray& ray ) const;
	void SetMaterial( const uint x, const uint y, const uint z, const unsigned short materialKey );

	// RT funstions
	float3 ShadowRay(Light const& light, float3 const& pixelWorldPos, float3 const& pixelNormal) const;

	// Managment functions
	bool AddLight(const Light& light);
	bool RemoveLight(const int id);

	vector<Light>& GetLights() { return lights; }
	map<unsigned short, Material>& GetMaterials() { return materials; }
	const Material& GetMaterialByKey(unsigned short key, bool& isKeyValid);

	vector<Light> lights;
	map<unsigned short, Material> materials;
	const Material defaultMaterial = Material(float3(0.75f, 0.0f, 0.75f), 0.0f, 0.0f);

	// grid contains key to a material in a map of materials;
	unsigned short *grid;

	//uint *grid;

private:
	bool Setup3DDDA( Ray& ray, DDAState& state ) const;
};

}