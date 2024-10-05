#include "template.h"

// -----------------------------------------------------------
// Calculate light transport via a ray
// -----------------------------------------------------------
float3 Renderer::Trace( Ray& ray, int, int, int /* we'll use these later */ )
{
	scene.FindNearest(ray);

	// Didn't find any voxel
	if (ray.voxelKey == NOMATERIALKEY) return GetSkyColor(ray);
	
	// Cancelling if voxel is behind the screen
	if (ray.t < 0) return GetSkyColor(ray);

	float3 N = ray.GetNormal();
	float3 I = ray.IntersectionPoint();

	bool isKeyValid = false;
	const Material& material = scene.GetMaterialByKey(ray.voxelKey, isKeyValid);

	//if (!isKeyValid) Do smth

	float3 albedo = material.albedo;

	float3 result = float3(0.0f);

	for (auto it = scene.GetLights().begin(); it != scene.GetLights().end(); it++)
	{
		result += albedo * scene.ShadowRay((*it), I, N);
	}

	return result;
}

float3 Tmpl8::Renderer::GetSkyColor(Ray& ray)
{
	return float3(0.4235f, 0.7255f, 0.9686f);
}

// -----------------------------------------------------------
// Application initialization - Executed once, at app start
// -----------------------------------------------------------
void Renderer::Init()
{
	dMousePos = int2{ 0, 0 };
	//scene.LoadLevelFromFile(levelFilepath);
}

// -----------------------------------------------------------
// Main application tick function - Executed every frame
// -----------------------------------------------------------
void Renderer::Tick( float deltaTime )
{
	// high-resolution timer, see template.h
	Timer t;
	// pixel loop: lines are executed as OpenMP parallel tasks (disabled in DEBUG)
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++)
	{
		// trace a primary ray for each pixel on the line
		for (int x = 0; x < SCRWIDTH; x++)
		{
			Ray r = camera.GetPrimaryRay( (float)x, (float)y );
			float3 pixel = Trace( r );
			screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8( pixel );
		}
	}
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf( "%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000 );
	// handle user input
	camera.HandleInput( deltaTime, dMousePos );
	dMousePos = int2{ 0, 0 };
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI()
{
	ImGui::Begin("Yes");
	ImGui::Image(GetRenderTargetPointer(), ImVec2(SCRWIDTH, SCRHEIGHT));
	ImGui::Text("something");
	ImGui::End();

	// ray query on mouse
	Ray r = camera.GetPrimaryRay((float)mousePos.x, (float)mousePos.y);
	scene.FindNearest(r);

	ImGui::Begin("Mouse and camera");

	ImGui::Text("Mouse hover: %i", r.voxelKey);

	ImGui::SliderFloat("Camera speed", &camera.speed, 0.0f, 0.002f, "%.5f");
	ImGui::SliderFloat("Camera sensivity", &camera.sensitivity, 0.0f, 0.02f);

	ImGui::End();




	vector<Light>& lights = scene.GetLights();

	ImGui::Begin("Scene");

	ImGui::InputText("", levelFilepath, 256);

	if (ImGui::Button("Load level from a file"))
		scene.LoadLevelFromFile(levelFilepath);

	ImGui::SameLine();

	if (ImGui::Button("Save level to a file"))
		scene.SaveLevelToFile(levelFilepath);

	if (ImGui::TreeNode("Lights"))
	{
		if (ImGui::BeginTable("Lights", 4, ImGuiTableFlags_Resizable))
		{
			for (int row = 0; row < lights.size(); row++)
			{
				ImGui::PushID(row);	// makes all the elements unique per row
				// Solves the issue with not-working buttons

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Checkbox("", &lights.at(row).isEnabled);

				ImGui::TableSetColumnIndex(1);
				ImGui::Text(lights.at(row).name);

				ImGui::TableSetColumnIndex(2);
				if (ImGui::Button("Select"))
				{
					selectedLightIndex = row;
				}

				ImGui::TableSetColumnIndex(3);
				if (ImGui::Button("Delete"))
				{
					scene.RemoveLight(row);

					if (selectedLightIndex == row) selectedLightIndex = -1;
					if (selectedLightIndex > row) selectedLightIndex -= 1;
				}

				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Materials"))
	{
		if (ImGui::BeginTable("Materials", 3, ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Albedo");

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Metalic");

			ImGui::TableSetColumnIndex(2);
			ImGui::Text("Roughness");

			auto& materials = scene.GetMaterials();

			for (auto it = materials.begin(); it != materials.end(); it++)
			{
				ImGui::TableNextRow();
				Material& material = it->second;

				ImGui::PushID(0); // Just something unique

				ImGui::TableSetColumnIndex(0);
				ImGui::ColorEdit3("", (float*) &material.albedo);

				ImGui::TableSetColumnIndex(1);
				ImGui::SliderFloat("", &material.metallic, 0.0f, 1.0f);

				ImGui::TableSetColumnIndex(2);
				ImGui::SliderFloat("", &material.roughness, 0.0f, 1.0f);

				ImGui::PopID();
			}			
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Create stuff"))
	{
		ImGui::Text("Create light source: ");

		ImGui::SameLine();
		if (ImGui::Button("Point"))
			scene.lights.push_back(Light::CreatePoint());

		ImGui::SameLine();
		if (ImGui::Button("Directional"))
			scene.lights.push_back(Light::CreateDirectional());

		ImGui::SameLine();
		if (ImGui::Button("Spot"))
			scene.lights.push_back(Light::CreateSpot());

		ImGui::SameLine();
		if (ImGui::Button("Area"))
			scene.lights.push_back(Light::CreateArea());
	}
	ImGui::End();	// "Scene" window


	bool areOperatingOnSelectedLight = false;
	if (selectedLightIndex != -1)
	{
		Light& selectedLight = lights.at(selectedLightIndex);
		areOperatingOnSelectedLight = true;

		LightType lightType = selectedLight.type;

		ImGui::Begin("Selected light", &areOperatingOnSelectedLight);

		switch (lightType)
		{
		case Point:
			ImGui::Text("Point");
			break;
		case Directional:
			ImGui::Text("Directional");
			break;
		case Spot:
			ImGui::Text("Spot");
			break;
		default:
			ImGui::Text("The light type is unknown!!");
			break;
		}

		ImGui::InputText("", selectedLight.name, 32);
		 
		ImGui::ColorEdit3("Color", (float*) (&selectedLight.color));

		if (lightType == Point || lightType == Spot)
			ImGui::DragFloat3("Position", (float*)(&selectedLight.pos), 0.001f);

		// Different thing then you think
		if (lightType == Directional || lightType == Spot)
			if (ImGui::DragFloat3("Direction", (float*)(&selectedLight.direction), 0.01f, 0.0f, 0.0f, "%.2f"))
				selectedLight.direction = normalize(selectedLight.direction);

		if (lightType == Spot)
		{
			ImGui::DragFloat("Inner cone cos angle", (&selectedLight.innerConeAngle));
			ImGui::DragFloat("Outer cone cos angle", (&selectedLight.outerConeAngle));
		}

		ImGui::DragFloat("Intensity", (&selectedLight.intensity), 0.001f);

		if (lightType == Spot || lightType == Point)
			ImGui::DragFloat("Range", (&selectedLight.range), 0.001f);

		ImGui::End();
	}

	if (!areOperatingOnSelectedLight) selectedLightIndex = -1;
}