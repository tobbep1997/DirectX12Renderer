#include "DirectX12Engine.h"

void CameraMovment(Camera * camera, const float & deltaTime)
{
	const float moveSpeed = 2.0f;
	const float rotSpeed = 2.0f;

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

	Camera * camera = new Camera(DirectX::XM_PI * 0.5, 16.0f / 9.0f, .1f, 100.0f);
	camera->SetPosition(0, 0, -5);

	DeltaTime deltaTimer;
	   
	StaticMesh * staticCylinderMesh = new StaticMesh();
	staticCylinderMesh->Init();
	staticCylinderMesh->LoadStaticMesh("../Models/Cylinder.fbx");

	StaticMesh * staticCubeMesh = new StaticMesh();
	staticCubeMesh->Init();
	staticCubeMesh->LoadStaticMesh("../Models/Cube.fbx");

	Drawable * drawable = new Drawable();
	drawable->SetPosition(-2, 0, 0);
	drawable->SetMesh(*staticCylinderMesh);
	drawable->Update();

	Drawable * drawable2 = new Drawable();
	drawable2->SetMesh(*staticCubeMesh);
	drawable2->Update();

	Texture * texture = new Texture();
	Texture * texture2 = new Texture();

	drawable->SetTexture(texture);
	drawable2->SetTexture(texture2);

	if(InitDirectX12Engine(window,
		renderingManager, 
		hInstance, 
		"Victor is Gay",
		1280, 
		720, 
		FALSE,
		TRUE))
	{
		staticCylinderMesh->CreateBuffer(renderingManager);
		staticCubeMesh->CreateBuffer(renderingManager);
		texture->LoadTexture("../car.bmp", renderingManager);
		texture2->LoadTexture("../car2.jpg", renderingManager);
		
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
	staticCylinderMesh->Release();
	staticCubeMesh->Release();
	renderingManager->Release(FALSE);
	texture->Release();
	texture2->Release();

	delete camera;
	delete staticCubeMesh;
	delete staticCylinderMesh;
	delete drawable;
	delete drawable2;
	delete texture;
	delete texture2;
	return 0;
}
