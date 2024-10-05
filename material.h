#pragma once

struct Material
{
	float3 albedo = float3(0.75f, 0.0f, 0.75f); // bright purple
	float metallic = 0.0f;
	float roughness = 1.0f;

	//Material() {};
	Material(float3 albedo_ = float3(0.75f, 0.0f, 0.75f), float metallic_ = 0.0f, float roughness_ = 0.0f)
	{
		albedo = albedo_; metallic = metallic_; roughness = roughness_;
	}

	// Here we can create a material by default
};