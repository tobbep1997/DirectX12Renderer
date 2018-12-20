#include "DirectX12Engine.h"

int WINAPI WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, 
	int nShowCmd)
{
	Window * window						= nullptr;
	RenderingManager * renderingManager = nullptr;

	if(InitDirectX12Engine(window,
		renderingManager, 
		hInstance, 
		"Victor is Gay",
		1280, 
		720, 
		FALSE,
		FALSE))
	{
		while (window->IsOpen())
		{
			if (!window->Updating())
			{
				
			}
			UpdateRenderingManger(renderingManager);
		}	
	}

	return 0;
}
