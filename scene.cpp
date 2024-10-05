#include "template.h"

inline float intersect_cube( Ray& ray )
{
	// branchless slab method by Tavian
	const float tx1 = -ray.O.x * ray.rD.x, tx2 = (1 - ray.O.x) * ray.rD.x;
	float ty, tz, tmin = min( tx1, tx2 ), tmax = max( tx1, tx2 );
	const float ty1 = -ray.O.y * ray.rD.y, ty2 = (1 - ray.O.y) * ray.rD.y;
	ty = min( ty1, ty2 ), tmin = max( tmin, ty ), tmax = min( tmax, max( ty1, ty2 ) );
	const float tz1 = -ray.O.z * ray.rD.z, tz2 = (1 - ray.O.z) * ray.rD.z;
	tz = min( tz1, tz2 ), tmin = max( tmin, tz ), tmax = min( tmax, max( tz1, tz2 ) );
	if (tmin == tz) ray.axis = 2; else if (tmin == ty) ray.axis = 1;
	return tmax >= tmin ? tmin : 1e34f;
}

inline bool point_in_cube( const float3& pos )
{
	// test if pos is inside the cube
	return pos.x >= 0 && pos.y >= 0 && pos.z >= 0 &&
		pos.x <= 1 && pos.y <= 1 && pos.z <= 1;
}

Scene::Scene()
{
	// Generate an emty grid
	grid = (unsigned short*)MALLOC64(GRIDSIZE3 * sizeof(unsigned short));
	memset(grid, 0, GRIDSIZE3 * sizeof(unsigned short));

	LoadDefaultLevel();
}

void Scene::LoadDefaultLevel()
{	
	// Creating a basic material of our level
	materials[1] = Material(
		float3(0.9f, 0.9f, 0.9f),
		0.0f, 
		1.0f
	);

	// initialize the scene using Perlin noise, parallel over z
#pragma omp parallel for schedule(dynamic)
	for (int z = 0; z < WORLDSIZE; z++)
	{
		const float fz = (float)z / WORLDSIZE;
		for (int y = 0; y < WORLDSIZE; y++)
		{
			float fx = 0, fy = (float)y / WORLDSIZE;
			for (int x = 0; x < WORLDSIZE; x++, fx += 1.0f / WORLDSIZE)
			{
				const float n = noise3D(fx, fy, fz);
				SetMaterial(x, y, z, n > 0.09f ? 1 : NOMATERIALKEY);
			}
		}
	}
}

bool Scene::LoadLevelFromFile(const char* filepath)
{
	// Loading a level from a file
	FILE* f = fopen(filepath, "rb");

	// Moving some data to heap for effiecency and to save stack
	SceneData* data = new SceneData();

	if (f)
	{
		fread(data, 1, sizeof(SceneData), f);
		fclose(f);
	}
	else
	{
		printf("Failed to load the level.");
		LoadDefaultLevel();
		delete data;
		return false;
	}

	materials.clear();
	for (uint i = 0; i < data->materialCount; i++)
	{
		materials[data->keysForMaterials[i]] = data->materials[i];
	}

	memset(grid, 0, GRIDSIZE3 * sizeof(unsigned short));

	// Let's not do it using memset, just afraid of doing something wrong
#pragma omp parallel for schedule(dynamic)
	for (int z = 0; z < WORLDSIZE; z++)
	{
		for (int y = 0; y < WORLDSIZE; y++)
		{
			for (int x = 0; x < WORLDSIZE; x++)
			{
				SetMaterial(x, y, z, data->grid[x + y * GRIDSIZE + z * GRIDSIZE2]);
			}
		}
	}

	lights.clear();
	for (uint i = 0; i < data->lightCount; i++)
	{
		lights.push_back(data->lights[i]);
	}

	delete data;

	return true;
}

bool Scene::SaveLevelToFile(const char* filepath)
{
	// Save a level to the file
	FILE* f = fopen(filepath, "wb");

	// Moving some data to heap for effiecency and to save stack
	SceneData* data = new SceneData();

	if (f)
	{
		// Let's not do it using memset, just afraid of doing something wrong
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < WORLDSIZE; z++)
		{
			for (int y = 0; y < WORLDSIZE; y++)
			{
				for (int x = 0; x < WORLDSIZE; x++)
				{
					data->grid[x + y * GRIDSIZE + z * GRIDSIZE2] = grid[x + y * GRIDSIZE + z * GRIDSIZE2];
				}
			}
		}

		data->lightCount = static_cast<unsigned int>(lights.size());

		for (uint i = 0; i < data->lightCount; i++)
		{
			data->lights[i] = lights.at(i);
		}

		data->materialCount = static_cast<unsigned int>(materials.size());
		
		if (data->materialCount > MAXMATERIALS)
		{
			printf("Material count is exceeding, not all the materials will be saved");
			data->materialCount = MAXMATERIALS;
		}

		auto it = materials.begin();

		for (uint i = 0; i < data->materialCount; i++, it++)
		{
			data->keysForMaterials[i] = it->first;
			data->materials[i] = it->second;
		}

		fwrite(data, 1, sizeof(SceneData), f);
		fclose(f);

		delete data;
		return true;
	}
	else
	{
		printf("Failed to save the level");
		delete data;
		return false;
	}
}

void Scene::SetMaterial( const uint x, const uint y, const uint z, const unsigned short materialKey)
{
	grid[x + y * GRIDSIZE + z * GRIDSIZE2] = materialKey;
}


bool Scene::Setup3DDDA( Ray& ray, DDAState& state ) const
{
	// if ray is not inside the world: advance until it is
	state.t = 0;
	bool startedInGrid = point_in_cube( ray.O );
	if (!startedInGrid)
	{
		state.t = intersect_cube( ray );
		if (state.t > 1e33f) return false; // ray misses voxel data entirely
	}
	// setup amanatides & woo - assume world is 1x1x1, from (0,0,0) to (1,1,1)
	static const float cellSize = 1.0f / GRIDSIZE;
	state.step = make_int3( 1 - ray.Dsign * 2 );
	const float3 posInGrid = GRIDSIZE * (ray.O + (state.t + 0.00005f) * ray.D);
	const float3 gridPlanes = (ceilf( posInGrid ) - ray.Dsign) * cellSize;
	const int3 P = clamp( make_int3( posInGrid ), 0, GRIDSIZE - 1 );
	state.X = P.x, state.Y = P.y, state.Z = P.z;
	state.tdelta = cellSize * float3( state.step ) * ray.rD;
	state.tmax = (gridPlanes - ray.O) * ray.rD;
	// detect rays that start inside a voxel
	uint cell = grid[P.x + P.y * GRIDSIZE + P.z * GRIDSIZE2];
	ray.inside = cell != 0 && startedInGrid;
	// proceed with traversal
	return true;
}

void Scene::FindNearest( Ray& ray ) const
{
	// nudge origin
	ray.O += EPSILON * ray.D;
	// setup Amanatides & Woo grid traversal
	DDAState s;
	if (!Setup3DDDA( ray, s )) return;
	//uint cell, lastCell = 0, axis = ray.axis;
	unsigned short cellKey, lastCellKey = NOMATERIALKEY, axis = ray.axis;
	if (ray.inside)
	{
		// start stepping until we find an empty voxel
		while (1)
		{
			cellKey = grid[s.X + s.Y * GRIDSIZE + s.Z * GRIDSIZE2];
			if (cellKey == NOMATERIALKEY) break;
			lastCellKey = cellKey;
			if (s.tmax.x < s.tmax.y)
			{
				if (s.tmax.x < s.tmax.z) { s.t = s.tmax.x, s.X += s.step.x, axis = 0; if (s.X >= GRIDSIZE) break; s.tmax.x += s.tdelta.x; }
				else { s.t = s.tmax.z, s.Z += s.step.z, axis = 2; if (s.Z >= GRIDSIZE) break; s.tmax.z += s.tdelta.z; }
			}
			else
			{
				if (s.tmax.y < s.tmax.z) { s.t = s.tmax.y, s.Y += s.step.y, axis = 1; if (s.Y >= GRIDSIZE) break; s.tmax.y += s.tdelta.y; }
				else { s.t = s.tmax.z, s.Z += s.step.z, axis = 2; if (s.Z >= GRIDSIZE) break; s.tmax.z += s.tdelta.z; }
			}
		}
		ray.voxelKey = lastCellKey; // we store the voxel we just left
	}
	else
	{
		// start stepping until we find a filled voxel
		while (1)
		{
			cellKey = grid[s.X + s.Y * GRIDSIZE + s.Z * GRIDSIZE2];
			if (cellKey != NOMATERIALKEY) break;
			else if (s.tmax.x < s.tmax.y)
			{
				if (s.tmax.x < s.tmax.z) { s.t = s.tmax.x, s.X += s.step.x, axis = 0; if (s.X >= GRIDSIZE) break; s.tmax.x += s.tdelta.x; }
				else { s.t = s.tmax.z, s.Z += s.step.z, axis = 2; if (s.Z >= GRIDSIZE) break; s.tmax.z += s.tdelta.z; }
			}
			else
			{
				if (s.tmax.y < s.tmax.z) { s.t = s.tmax.y, s.Y += s.step.y, axis = 1; if (s.Y >= GRIDSIZE) break; s.tmax.y += s.tdelta.y; }
				else { s.t = s.tmax.z, s.Z += s.step.z, axis = 2; if (s.Z >= GRIDSIZE) break; s.tmax.z += s.tdelta.z; }
			}
		}
		ray.voxelKey = cellKey;
	}
	ray.t = s.t;
	ray.axis = axis;
}

bool Scene::IsOccluded( Ray& ray ) const
{
	// nudge origin
	ray.O += EPSILON * ray.D;
	ray.t -= EPSILON * 2.0f;
	// setup Amanatides & Woo grid traversal
	DDAState s;
	if (!Setup3DDDA( ray, s )) return false;
	// start stepping
	while (s.t < ray.t)
	{
		const uint cell = grid[s.X + s.Y * GRIDSIZE + s.Z * GRIDSIZE2];
		if (cell) /* we hit a solid voxel */ return s.t < ray.t;
		if (s.tmax.x < s.tmax.y)
		{
			if (s.tmax.x < s.tmax.z) { if ((s.X += s.step.x) >= GRIDSIZE) return false; s.t = s.tmax.x, s.tmax.x += s.tdelta.x; }
			else { if ((s.Z += s.step.z) >= GRIDSIZE) return false; s.t = s.tmax.z, s.tmax.z += s.tdelta.z; }
		}
		else
		{
			if (s.tmax.y < s.tmax.z) { if ((s.Y += s.step.y) >= GRIDSIZE) return false; s.t = s.tmax.y, s.tmax.y += s.tdelta.y; }
			else { if ((s.Z += s.step.z) >= GRIDSIZE) return false; s.t = s.tmax.z, s.tmax.z += s.tdelta.z; }
		}
	}
	return false;
}


float3 Scene::ShadowRay(Light const& light, float3 const& pixelWorldPos, float3 const& pixelNormal) const
{
	if (!light.isEnabled) return float3(0.0f);

	switch (light.type)
	{
	case LightType::Point:
	{
		float3 lightToPixelVector = pixelWorldPos - light.pos;
		float3 direction = normalize(lightToPixelVector);
		float rayLength = length(lightToPixelVector);

		if (rayLength > light.range) return float3(0.0f);

		Ray r(light.pos, direction, rayLength);

		if (IsOccluded(r))
			return float3(0.0f);
		else
		{
			float3 result = light.color;
			result *= (light.range - rayLength) / light.range;
			result *= light.intensity;	/// REDO, should be something else
			result *= dot(pixelNormal, -direction);

			return result;
		}
	}

	break;

	case LightType::Directional:
	{
		// Casting an opposite ray, as it is the same
		Ray r(pixelWorldPos, -light.direction);

		if (IsOccluded(r))
			return float3(0.0f);
		else
		{
			float3 result = light.color;
			result *= light.intensity;	/// REDO, should be something else
			result *= dot(pixelNormal, -light.direction);

			return result;
		}
	}

	break;

	case LightType::Spot:

		printf("We don't support spot light yet");
		abort();
		return float3(0.0f);
		break;
	default:
		printf("The light type is unknown");
		abort();
		return float3(0.0f);
		break;
	}
}

bool Scene::AddLight(const Light& light)
{
	lights.push_back(light);

	return true;
}

bool Scene::RemoveLight(const int id)
{
	lights.erase(lights.begin() + id);

	return true;
}

const Material& Tmpl8::Scene::GetMaterialByKey(unsigned short key, bool& isKeyValid)
{
	// Shoot ourselves for such a hillarious mistake
	if (key == NOMATERIALKEY) abort();

	auto it = materials.find(key);
	bool materialWasNotFound = it == materials.end();
	if (materialWasNotFound) { isKeyValid = false; return defaultMaterial; }

	isKeyValid = true;
	return it->second;
}
