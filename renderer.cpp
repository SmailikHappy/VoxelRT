#include "template.h"

// -----------------------------------------------------------
// Calculate light transport via a ray
// -----------------------------------------------------------
float3 Renderer::Trace( Ray& ray, int, int, int /* we'll use these later */ )
{
	scene.FindNearest(ray);

	// Didn't find any voxel
	if (ray.voxel == 0) return GetSkyColor(ray);
	
	// Cancelling if voxel is behind the screen
	if (ray.t < 0) return GetSkyColor(ray);

	float3 N = ray.GetNormal();
	float3 I = ray.IntersectionPoint();
	float3 albedo = ray.GetAlbedo();

	float3 result = float3(0.0f);

	for (auto it = scene.lightHandler.get()->GetLights().begin(); it != scene.lightHandler.get()->GetLights().end(); it++)
	{
		result += albedo * scene.lightHandler.get()->ShadowRay((*it), I, N);
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

	scene.lightHandler.get()->AddLight(Light::CreatePoint(float3(0.001f, 0.001f, 0.001f), 1.0f, float3(1), 1));
	scene.lightHandler.get()->AddLight(Light::CreateDirectional(float3(-0.5f, -0.6f, -0.7f), float3(0.7f, 0.85f, 1.0f), 2.0f));
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
	// ray query on mouse
	Ray r = camera.GetPrimaryRay( (float)mousePos.x, (float)mousePos.y );
	scene.FindNearest( r );

	ImGui::Begin("MouseInfo");
	ImGui::Text( "voxel: %i", r.voxel );
	ImGui::End();


	ImGui::Begin("Scene");
	ImGui::Text("Lights");
	if (ImGui::BeginTable("Lights", 3))
	{
		for (int row = 0; row < scene.lightHandler.get()->GetLights().size(); row++)
		{
			ImGui::TableNextRow();
			
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Light numero " + row);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Light selecto");

			ImGui::TableSetColumnIndex(2);
			ImGui::Text("Light deleto");
		}

		ImGui::EndTable();
	}
	ImGui::End();
}