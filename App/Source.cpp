#include "DirectX12Engine.h"

int WINAPI WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, 
	int nShowCmd)
{
	Window * window						= nullptr;
	RenderingManager * renderingManager = nullptr;

	Camera * camera = new Camera();
	camera->SetPosition(0, 0, -5);

	DeltaTime deltaTimer;

	StaticMesh * staticMesh = new StaticMesh();
	staticMesh->LoadStaticMesh("../Models/Cube.fbx");

	Drawable * drawable = new Drawable();
	drawable->SetMesh(*staticMesh);

	if(InitDirectX12Engine(window,
		renderingManager, 
		hInstance, 
		"Victor is Gay",
		1280, 
		720, 
		FALSE,
		TRUE))
	{
		deltaTimer.Init();
		while (window->IsOpen())
		{
			const double deltaTime = deltaTimer.GetDeltaTimeInSeconds();
			if (window->Updating())
			{
			}

			if (Input::IsKeyPressed('A'))
				camera->Translate(static_cast<float>(-1.0 * deltaTime), 0, 0);
			if (Input::IsKeyPressed('D'))
				camera->Translate(static_cast<float>(1.0 * deltaTime), 0, 0);
			if (Input::IsKeyPressed('W'))
				camera->Translate(0, 0, static_cast<float>(1.0 * deltaTime));
			if (Input::IsKeyPressed('S'))
				camera->Translate(0, 0, static_cast<float>(-1.0 * deltaTime));


			camera->Update();
			UpdateRenderingManger(renderingManager, *camera);
		}	
	}

	delete camera;
	delete staticMesh;

	return 0;
}
