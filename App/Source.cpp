#include "DirectX12Engine.h"
#include "Utility/DeltaTime.h"

void CameraMovment(Camera * camera, const float & deltaTime)
{

	float sprintMod = 0.5f;
	if (Input::IsKeyPressed(16))
		sprintMod = 4.0f;
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

	Camera * camera = new Camera(DirectX::XM_PI * 0.5, 16.0f / 9.0f, .01f, 100.0f);
	camera->SetPosition(0, 0, -5);

	DeltaTime deltaTimer;
	   
	StaticMesh * staticCylinderMesh = new StaticMesh();
	staticCylinderMesh->Init();
	staticCylinderMesh->LoadStaticMesh("../Models/Sphere.fbx");

	StaticMesh * staticCubeMesh = new StaticMesh();
	staticCubeMesh->Init();
	staticCubeMesh->LoadStaticMesh("../Models/Cube.fbx");

	Texture * texture = new Texture();
	Texture * normal = new Texture();
	Texture * metallic = new Texture();
	Texture * displacement = new Texture();
	

	Drawable * drawable = new Drawable();
	drawable->SetPosition(0, 0, 0);
	drawable->SetScale(1, 1, 1);
	drawable->SetMesh(*staticCylinderMesh);
	drawable->Update();
	drawable->SetTexture(texture);
	drawable->SetNormalMap(normal);
	drawable->SetMetallicMap(metallic);
	drawable->SetDisplacementMap(displacement);

	Drawable * floor = new Drawable();
	floor->SetPosition(0, -2, 0);
	floor->SetScale(50, 1, 50);
	floor->SetMesh(*staticCubeMesh);
	floor->Update();
	floor->SetTexture(texture);
	floor->SetNormalMap(normal);
	floor->SetMetallicMap(metallic);
	floor->SetDisplacementMap(displacement);

	const UINT pointLightSize = 1;
	std::vector<PointLight*> pointLights = std::vector<PointLight*>(pointLightSize);
	for (UINT i = 0; i < pointLightSize; i++)
	{
		pointLights[i] = new PointLight();
		pointLights[i]->SetPosition(0, 0, -2.5);  
		pointLights[i]->SetIntensity(2.5);  
	}

	DirectionalLight* directionalLight = new DirectionalLight();
	directionalLight->SetPosition(0, 5, 0);
	directionalLight->SetDirection(0, -1, 0);
	directionalLight->GetCamera()->SetUp(0, 0, 1);
	directionalLight->GetCamera()->Update();
	directionalLight->SetIntensity(.5f);

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
		staticCubeMesh->CreateBuffer(renderingManager);
		texture->LoadTexture("../Texture/Brick/Brick_diffuse.bmp", renderingManager);
		normal->LoadTexture("../Texture/Brick/Brick_normal.bmp", renderingManager);
		metallic->LoadTexture("../Texture/Brick/Brick_metallic.bmp", renderingManager);
		displacement->LoadTexture("../Texture/Brick/Brick_height.bmp", renderingManager);
		
		deltaTimer.Init();
		while (Window::IsOpen())
		{
			const float deltaTime = static_cast<const float>(deltaTimer.GetDeltaTimeInSeconds());
			if (Window::Updating())
			{
			}

			CameraMovment(camera, deltaTime);
			camera->Update();

			drawable->SetRotation(0, drawable->GetRotation().y + deltaTime * 0.25f, 0);
			drawable->Update();
			drawable->Draw(renderingManager);
			floor->Draw(renderingManager);
			directionalLight->Queue(renderingManager);
			for (UINT i = 0; i < pointLightSize; i++)
			{
				pointLights[i]->Queue(renderingManager);
			}
		
			UpdateRenderingManger(renderingManager, *camera);
			if (Input::IsKeyPressed('P'))
				RestartRenderingManager(window, renderingManager, TRUE);
		}	
	}
	renderingManager->WaitForFrames();
	staticCylinderMesh->Release();
	staticCubeMesh->Release();
	texture->Release();
	normal->Release();
	metallic->Release();
	displacement->Release();
	directionalLight->Release();
	renderingManager->Release(FALSE);

	delete camera;
	delete staticCylinderMesh;
	delete staticCubeMesh;
	delete drawable;
	delete floor;
	delete texture;
	delete normal;
	delete metallic;
	delete displacement;
	delete directionalLight;
	for (UINT i = 0; i < pointLightSize; i++)
		delete pointLights[i];
	return 0;
}
