#include "template.h"
#include "lighthandler.h"
//#include "scene.h"


float3 LightHandler::ShadowRay(unique_ptr<Light> const& light, float3 const& pixelWorldPos, float3 const& pixelNormal) const
{
	if (!light.get()->isEnabled) return float3(0.0f);

	switch (light.get()->type)
	{
	case LightType::Point:
	{
		float3 lightToPixelVector = pixelWorldPos - light.get()->pos;
		float3 direction = normalize(lightToPixelVector);
		float rayLength = length(lightToPixelVector);

		Ray r(light.get()->pos, direction, rayLength);

		if (scene->IsOccluded(r))
			return float3(0.0f);
		else
		{
			float3 result = light.get()->color;
			result *= (light.get()->range - rayLength) / light.get()->range;
			result *= light.get()->intensity;	/// REDO, should be something else
			result *= dot(pixelNormal, -direction);

			return result;
		}
	}

	break;

	case LightType::Directional:
	{
		// Casting an opposite ray, as it is the same
		Ray r(pixelWorldPos, -light.get()->direction);

		if (scene->IsOccluded(r))
			return float3(0.0f);
		else
		{
			float3 result = light.get()->color;
			result *= light.get()->intensity;	/// REDO, should be something else
			result *= dot(pixelNormal, -light.get()->direction);

			return result;
		}
	}

	break;

	case LightType::Spot:

		printf("We don't support spot light yet");
		return float3(0.0f);
		break;
	default:
		printf("The light type is unknown");
		return float3(0.0f);
		break;
	}
}

bool LightHandler::AddLight(const Light& light)
{
	lights.push_back(make_unique<Light>(light));

	return true;
}

bool LightHandler::RemoveLight(vector<unique_ptr<Light>>::iterator it)
{
	lights.erase(it);

	return true;
}

bool LightHandler::RemoveLight(const int id)
{
	lights.erase(lights.begin() + id);

	return true;
}
