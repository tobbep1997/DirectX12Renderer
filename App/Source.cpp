#include "DirectX12Engine.h"

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )

void _CrtSetDbgFlag()
{
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

void _AlocConsole() {
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
}

#else
#define DBG_NEW new
#endif


int WINAPI WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, 
	int nShowCmd)
{
#ifdef _DEBUG
	_CrtSetDbgFlag();
	_AlocConsole();
#endif

	Window * window						= nullptr;
	RenderingManager * renderingManager = nullptr;

	if(InitDirectX12Engine(window,
		renderingManager, 
		hInstance, 
		"DirectX12 Ejaculant",
		1280, 
		720, 
		FALSE))
	{
		while (window->IsOpen())
		{
			if (!window->Updating())
			{
				//RUN GAME HERE
			}
			renderingManager->Flush();
			renderingManager->Present();
		}	
	}

	renderingManager->Release();
	return 0;
}
