#include "DirectX12Engine.h"

void CameraMovement(Camera * camera, const float & deltaTime)
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

	if(InitDirectX12Engine(window,
		renderingManager, 
		hInstance, 
		"DirectX Renderer",
		1280, 
		720, 
		FALSE,
		TRUE,
		FALSE))
	{
		Camera * camera = new Camera(DirectX::XM_PI * 0.5, 16.0f / 9.0f, .01f, 100.0f);
		camera->SetPosition(0, 0, -5);

		DeltaTime deltaTimer;

		StaticMesh * staticCylinderMesh = new StaticMesh();
		staticCylinderMesh->Init();
		staticCylinderMesh->LoadStaticMesh("../Models/Cylinder.fbx");

		StaticMesh * staticCubeMesh = new StaticMesh();
		staticCubeMesh->Init();
		staticCubeMesh->LoadStaticMesh("../Models/Cube.fbx");

		Texture * texture = new Texture();
		Texture * normal = new Texture();
		Texture * metallic = new Texture();
		Texture * displacement = new Texture();


		Drawable * drawable = new Drawable();
		drawable->SetPosition(0, 0, 0);
		drawable->SetScale(1, .2, 1);
		drawable->SetMesh(*staticCylinderMesh);
		drawable->Update();
		drawable->SetTexture(texture);
		drawable->SetNormalMap(normal);
		drawable->SetMetallicMap(metallic);
		drawable->SetDisplacementMap(displacement);

		const int cubesSize = 16;
		std::vector<Drawable*> cubes = std::vector<Drawable*>(cubesSize);
		for (UINT i = 0; i < cubesSize; i++)
		{
			cubes[i] = new Drawable();
			cubes[i]->SetPosition(static_cast<float>(rand() % cubesSize) - (cubesSize / 2), 0.0f, static_cast<float>(rand() % cubesSize) - (cubesSize / 2)); //NOLINT
			cubes[i]->SetScale(1, 1, 1);
			cubes[i]->SetMesh(*staticCubeMesh);
			cubes[i]->Update();
			cubes[i]->SetTexture(texture);
			cubes[i]->SetNormalMap(normal);
			cubes[i]->SetMetallicMap(metallic);
			cubes[i]->SetDisplacementMap(displacement);
		}


		const int floorSize = 20;

		Drawable * floor = new Drawable();
		floor->SetPosition(0, -2, 0);
		floor->SetScale(floorSize, 1, floorSize);
		floor->SetMesh(*staticCubeMesh);
		floor->Update();
		floor->SetTexture(texture);
		floor->SetNormalMap(normal);
		floor->SetMetallicMap(metallic);
		floor->SetDisplacementMap(displacement);

		Texture * fire1 = new Texture();
		Texture * fire2 = new Texture();
		Texture * fire3 = new Texture();

		staticCubeMesh->CreateBuffer();
		staticCylinderMesh->CreateBuffer();
		texture->LoadDDSTexture("../Texture/Brick/Brick_diffuse.DDS", TRUE);
		normal->LoadDDSTexture("../Texture/Brick/Brick_normal.DDS", TRUE);
		metallic->LoadDDSTexture("../Texture/Brick/Brick_metallic.DDS", TRUE);
		displacement->LoadDDSTexture("../Texture/Brick/Brick_height.DDS", TRUE);

		fire1->LoadTexture("../Texture/Fire/Fire1.bmp", FALSE);
		fire2->LoadTexture("../Texture/Fire/Fire2.bmp", FALSE);
		fire3->LoadTexture("../Texture/Fire/Fire3.bmp", FALSE);

		const int pointLightSize = 32;
		std::vector<PointLight*> pointLights = std::vector<PointLight*>(pointLightSize);
		for (UINT i = 0; i < pointLightSize; i++)
		{
			pointLights[i] = new PointLight(renderingManager, *window);
			pointLights[i]->Init();
			pointLights[i]->SetPosition((rand() % floorSize) - (floorSize / 2), 3, (rand() % floorSize) - (floorSize / 2));
			//pointLights[i]->SetPosition(0,2,0);
			pointLights[i]->SetIntensity(15.5f);
			pointLights[i]->SetDropOff(2.0f);
			pointLights[i]->SetPow(1.5f);
			pointLights[i]->SetRadius(7.5f);
			pointLights[i]->SetColor(1, 1, 1);
			pointLights[i]->Update();
			pointLights[i]->SetCastShadows(true);
		}

		DirectionalLight* directionalLight = new DirectionalLight(renderingManager, *window);
		directionalLight->Init();
		directionalLight->SetPosition(0, 5, 0);
		directionalLight->SetDirection(0, 0, 0);
		directionalLight->GetCamera()->SetFocusPoint(TRUE);
		directionalLight->GetCamera()->SetUp(1, 0, 0);
		directionalLight->GetCamera()->Update();
		directionalLight->SetIntensity(0.4f);


		const UINT MAX_EMITTERS = 64;
		ParticleEmitter * emitter[MAX_EMITTERS];
		
		Texture* emitterTextures[3] = { fire1, fire2, fire3 };
		
		for (UINT i = 0; i < MAX_EMITTERS; i++)
		{
			emitter[i] = new ParticleEmitter(
				*window,
				256,
				256,
				3,
				DXGI_FORMAT_B8G8R8X8_UNORM);

			emitter[i]->SetTextures(emitterTextures);
			emitter[i]->Init();
			emitter[i]->SetPosition(0, .25f, 0);
			emitter[i]->SetPosition(10, .25f, 0);
			emitter[i]->SetPosition((rand() % floorSize) - (floorSize / 2), 3, (rand() % floorSize) - (floorSize / 2));
			emitter[i]->Update();
		}

		deltaTimer.Init();
		while (Window::IsOpen())
		{
			const float deltaTime = static_cast<const float>(deltaTimer.GetDeltaTimeInSeconds());
			if (Window::Updating())
			{
			}
			
			CameraMovement(camera, deltaTime);
			camera->Update();

			drawable->SetRotation(0, drawable->GetRotation().y + deltaTime * 0.25f, 0);
			drawable->Update();
			drawable->Draw();
			floor->Draw();

			for (UINT i = 0; i < cubesSize; i++)
			{
				cubes[i]->SetRotation(0, cubes[i]->GetRotation().y + deltaTime * 0.25f, 0);
				cubes[i]->Update();
				cubes[i]->Draw();
			}


			directionalLight->Queue();
			for (UINT i = 0; i < pointLightSize; i++)
			{
				pointLights[i]->Queue();
			}
			//directionalLight2->Queue();

			for (UINT i = 0; i < MAX_EMITTERS; i++)
			{
				emitter[i]->Draw();
			}

			if (Input::IsKeyPressed(97))
				directionalLight->SetPosition(5, 5, 5);
			else if (Input::IsKeyPressed(98))
				directionalLight->SetPosition(0, 5, 5);
			else if (Input::IsKeyPressed(99))
				directionalLight->SetPosition(-5, 5, 5);
			else if (Input::IsKeyPressed(100))
				directionalLight->SetPosition(5, 5, 0);
			else if (Input::IsKeyPressed(101))
				directionalLight->SetPosition(0, 5, 0);
			else if (Input::IsKeyPressed(102))
				directionalLight->SetPosition(-5, 5, 0);
			else if (Input::IsKeyPressed(103))
				directionalLight->SetPosition(5, 5, -5);
			else if (Input::IsKeyPressed(104))
				directionalLight->SetPosition(0, 5, -5);
			else if (Input::IsKeyPressed(105))
				directionalLight->SetPosition(-5, 5, -5);	
			directionalLight->GetCamera()->Update();

		
			UpdateRenderingManger(renderingManager, deltaTime, camera);
			if (Input::IsKeyPressed('P'))
				RestartRenderingManager(window, renderingManager, FALSE);
		}	
		renderingManager->WaitForFrames();
		directionalLight->Release();
		for (UINT i = 0; i < pointLightSize; i++)
			pointLights[i]->Release();

		delete directionalLight;
		for (UINT i = 0; i < pointLightSize; i++)
			delete pointLights[i];

		for (UINT i = 0; i < MAX_EMITTERS; i++)
		{
			emitter[i]->Release();
			SAFE_DELETE(emitter[i]);
		}

		staticCylinderMesh->Release();
		staticCubeMesh->Release();
		texture->Release();
		normal->Release();
		metallic->Release();
		displacement->Release();
		fire1->Release();
		fire2->Release();
		fire3->Release();
		floor->Release();
		drawable->Release();
		renderingManager->Release(FALSE);

		SAFE_DELETE(camera);
		SAFE_DELETE(staticCylinderMesh);
		SAFE_DELETE(staticCubeMesh);
		SAFE_DELETE(drawable);
		for (UINT i = 0; i < cubesSize; i++)
		{
			delete cubes[i];
		}
		SAFE_DELETE(floor);
		SAFE_DELETE(texture);
		SAFE_DELETE(normal);
		SAFE_DELETE(metallic);
		SAFE_DELETE(displacement);

		SAFE_DELETE(fire1);
		SAFE_DELETE(fire2);
		SAFE_DELETE(fire3);
	}



	return 0;
}
