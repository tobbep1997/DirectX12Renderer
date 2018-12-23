#include "DirectX12Engine.h"

void CameraMovment(Camera * camera, const float & deltaTime)
{
	const float moveSpeed = 1.0f;
	const float rotSpeed = 1.0f;

	if (Input::IsKeyPressed('A'))
		camera->Translate(-moveSpeed * deltaTime, 0, 0);
	else if (Input::IsKeyPressed('D'))
		camera->Translate(moveSpeed * deltaTime, 0, 0);

	if (Input::IsKeyPressed('W'))
		camera->Translate(0, 0, moveSpeed * deltaTime);
	else if (Input::IsKeyPressed('S'))
		camera->Translate(0, 0, -moveSpeed * deltaTime);

	if (Input::IsKeyPressed(Input::Keys::LeftArrow))
		camera->Rotate(0, -rotSpeed * deltaTime, 0);
	else if (Input::IsKeyPressed(Input::Keys::RightArrow))
		camera->Rotate(0, rotSpeed * deltaTime, 0);

	if (Input::IsKeyPressed(Input::Keys::UpArrow))
		camera->Rotate(-rotSpeed * deltaTime, 0, 0);
	else if (Input::IsKeyPressed(Input::Keys::DownArrow))
		camera->Rotate(rotSpeed * deltaTime, 0, 0);
}

int WINAPI WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, 
	int nShowCmd)
{
	Window * window = nullptr;
	RenderingManager * renderingManager = nullptr;

	Camera * camera = new Camera();
	camera->SetPosition(0, 0, -5);

	DeltaTime deltaTimer;

	StaticMesh * staticCubeMesh = new StaticMesh();
	staticCubeMesh->Init();
	staticCubeMesh->LoadStaticMesh("../Models/Cube.fbx");

	StaticMesh * staticCylinderMesh = new StaticMesh();
	staticCylinderMesh->Init();
	staticCylinderMesh->LoadStaticMesh("../Models/Cylinder.fbx");

	Drawable * drawable = new Drawable();
	drawable->SetMesh(*staticCylinderMesh);
	drawable->SetPosition(-1.f, 0, 0);
	Drawable * drawable2 = new Drawable();
	drawable2->SetMesh(*staticCubeMesh);
	drawable2->SetPosition(1.f, 0, 0);

	drawable->Update();
	drawable2->Update();

	if(InitDirectX12Engine(window,
		renderingManager, 
		hInstance, 
		"Victor is Gay",
		1280, 
		720, 
		FALSE,
		TRUE))
	{
		staticCubeMesh->CreateBuffer(renderingManager);
		staticCylinderMesh->CreateBuffer(renderingManager);

		deltaTimer.Init();
		while (window->IsOpen())
		{
			const float deltaTime = static_cast<const float>(deltaTimer.GetDeltaTimeInSeconds());
			if (window->Updating())
			{
			}

			CameraMovment(camera, deltaTime);


			camera->Update();
			drawable->Draw(renderingManager);
			drawable2->Draw(renderingManager);
			UpdateRenderingManger(renderingManager, *camera);
		}	
	}
	renderingManager->WaitForFrames();
	staticCubeMesh->Release();
	staticCylinderMesh->Release();
	renderingManager->Release(FALSE);

	delete camera;
	delete staticCubeMesh;
	delete staticCylinderMesh;
	delete drawable;
	delete drawable2;
	return 0;
}
