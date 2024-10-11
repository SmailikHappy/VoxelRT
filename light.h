#pragma once

enum LightType
{
	Point,
	Directional,
	Spot,
	Area,
	Unknown
};

struct Light
{
	LightType type = LightType::Unknown;
	char name[32] = "name";
	float3 pos = float3(0.0f, 0.0f, 0.0f);
	float3 direction = float3(0.0f, 0.0f, 1.0f);
	float range = 1.0f;
	float3 color = float3(1.0f, 0.0f, 1.0f);
	float intensity = 1.0f;
	float innerConeAngle = 0.0f;	// for spot only
	float outerConeAngle = 1.0f;	// for spot only
	bool isEnabled = true;

	static float3 GetLightVector(float3 pixelPos, const Light& light)
	{
		switch (light.type)
		{
		case Point:
			return pixelPos - light.pos;
			
		case Directional:
			return light.direction;

		case Spot:
			return pixelPos - light.pos;

		case Area:
			return float3(0.0f);

		case Unknown:
			return float3(0.0f);

		default:
			break;
		}
	}

	static Light CreatePoint()
	{
		Light light;
		light.type = LightType::Point;
		strcpy(light.name, "Point light source");

		// All other variables are already on some defaults

		return light;
	}

	static Light CreateDirectional()
	{
		Light light;
		light.type = LightType::Directional;
		strcpy(light.name, "Directional light source");


		// All other variables are already on some defaults

		return light;
	}

	static Light CreateSpot()
	{
		Light light;
		light.type = LightType::Spot;
		strcpy(light.name, "Spot light source");

		// All other variables are already on some defaults

		return light;
	}

	static Light CreateArea()
	{
		Light light;
		light.type = LightType::Area;
		strcpy(light.name, "Area light source");

		// All other variables are already on some defaults

		return light;
	}

	static Light CreatePoint(float3 const &_pos, const float _range, float3 const& _color, const float _intensity)
	{
		Light light;
		light.type = LightType::Point;
		light.pos = _pos;
		light.range = _range;
		light.color = _color;
		light.intensity = _intensity;

		return light;
	}

	static Light CreateDirectional(float3 const& _direction, float3 const& _color, const float _intensity)
	{
		Light light;
		light.type = LightType::Directional;
		light.direction = _direction;
		light.color = _color;
		light.intensity = _intensity;

		return light;
	}

	static Light CreateSpot(float3 const& _pos, float3 const& _direction, const float _range, float3 const& _color, const float _intensity, const float _innerConeAngle, const float _outerConeAngle)
	{
		Light light;
		light.type = LightType::Spot;
		light.pos = _pos;
		light.direction = _direction;
		light.range = _range;
		light.color = _color;
		light.intensity = _intensity;
		light.innerConeAngle = _innerConeAngle;
		light.outerConeAngle = _outerConeAngle;

		return light;
	}
};