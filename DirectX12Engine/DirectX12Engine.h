#pragma once
#include "DirectX12EnginePCH.h"
#include "Utility/DeltaTime.h"
#include "DirectX/Objects/ParticleEmitter.h"


#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define DBG_NEW new
#endif

namespace DEBUG //NOLINT
{
	

	inline void SetDbgFlag()
	{
	#ifdef _DEBUG
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif
	}

	inline void AlocConsole() {
	#ifdef _DEBUG
		AllocConsole();
		FILE* fp;
		freopen_s(&fp, "CONIN$", "r", stdin);
		freopen_s(&fp, "CONOUT$", "w", stdout);
	#endif
	}

}
inline BOOL InitDirectX12Engine(Window *& window,
	RenderingManager *& renderingManager,
	const HINSTANCE hInstance,					// NOLINT
	const std::string & windowName,  
	const UINT & width,
	const UINT & height,
	const BOOL & fullscreen = FALSE,
	const BOOL & debuggingTools = FALSE,
	const BOOL & enableConsole = FALSE)
{
	DEBUG::SetDbgFlag();
	if (enableConsole)
		DEBUG::AlocConsole();

	HRESULT hr;
	if (window || renderingManager)
		return FALSE;

	window = Window::GetInstance();
	renderingManager = RenderingManager::GetInstance();

	PRINT("Init Window Start") NEW_LINE;
	if (SUCCEEDED(hr = window->Create(hInstance, windowName, width, height, fullscreen)))
	{
		PRINT("Init Window Done") NEW_LINE;
		PRINT("Init DirectX12Engine Start") NEW_LINE;
		if (SUCCEEDED(hr = renderingManager->Init(window, debuggingTools)))
		{
			PRINT("Init DirectX12Engine Done") NEW_LINE;
			return TRUE;
		}
	}
	OutputDebugString("DirectX12 FAILED\n");
	
	return FALSE;
}

inline void UpdateRenderingManger(RenderingManager *& renderingManager, const float & deltaTime, const Camera * camera, const BOOL & present = TRUE)
{
	if (renderingManager)
		renderingManager->Flush(camera, deltaTime, present);
	
}

inline BOOL RestartRenderingManager(Window *& window, RenderingManager *& renderingManager, const BOOL & debuggingTools = FALSE)
{
	HRESULT hr;

	if (!window || !renderingManager)
		return FALSE;

	renderingManager->Release(TRUE, FALSE);

	if (SUCCEEDED(hr = renderingManager->Init(window, debuggingTools)))
	{
		OutputDebugString("DirectX12 Restart\n");
		return TRUE;
	}
	OutputDebugString("DirectX12 FAILED to Restart\n");

	return FALSE;
}