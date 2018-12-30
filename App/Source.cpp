#include "DirectX12Engine.h"

void CameraMovment(Camera * camera, const float & deltaTime)
{

	float sprintMod = 1.0f;
	if (Input::IsKeyPressed(16))
		sprintMod = 2.0f;
	const float moveSpeed = 2.0f * sprintMod;
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
	staticCylinderMesh->LoadStaticMesh("../Models/Cube.fbx");

	Texture * texture = new Texture();
	Texture * normal = new Texture();
	

	Drawable * drawable = new Drawable();
	drawable->SetPosition(0, 0, 0);
	drawable->SetScale(5, 5, 5);
	drawable->SetMesh(*staticCylinderMesh);
	drawable->Update();
	drawable->SetTexture(texture);
	drawable->SetNormalMap(normal);


	if(InitDirectX12Engine(window,
		renderingManager, 
		hInstance, 
		"Victor is Gay",
		1280, 
		720, 
		FALSE,
		TRUE,
		FALSE))
	{
		staticCylinderMesh->CreateBuffer(renderingManager);
		texture->LoadTexture("../Texture/Bark/Bark_diffuse.bmp", renderingManager);
		normal->LoadTexture("../Texture/Bark/Bark_normal.bmp", renderingManager);
		
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
			UpdateRenderingManger(renderingManager, *camera);
		
			if (Input::IsKeyPressed('P'))
				RestartRenderingManager(window, renderingManager, TRUE);
		}	
	}
	renderingManager->WaitForFrames();
	staticCylinderMesh->Release();
	texture->Release();
	normal->Release();
	renderingManager->Release(FALSE);

	delete camera;
	delete staticCylinderMesh;
	delete drawable;
	delete texture;
	delete normal;
	return 0;
}
