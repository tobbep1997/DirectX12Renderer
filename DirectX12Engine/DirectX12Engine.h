#pragma once
#include "DirectX12EnginePCH.h"

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define DBG_NEW new
#endif


inline void _CrtSetDbgFlag()
{
#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

inline void _AlocConsole() {
#ifdef _DEBUG
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
#endif
}

inline BOOL InitDirectX12Engine(Window *& window,
	RenderingManager *& renderingManager,
	HINSTANCE hInstance,
	const std::string & windowName,
	const UINT & width,
	const UINT & height,
	const BOOL & fullscreen = FALSE,
	const BOOL & debuggingTools = FALSE)
{
	_CrtSetDbgFlag();
	if (debuggingTools)
		_AlocConsole();

	HRESULT hr;
	if (window || renderingManager)
		return E_INVALIDARG;

	window = Window::GetInstance();
	renderingManager = RenderingManager::GetInstance();

	if (SUCCEEDED(hr = window->Create(hInstance, windowName, width, height, fullscreen)))
	{
		if (SUCCEEDED(hr = renderingManager->Init(*window, debuggingTools)))
		{
			return TRUE;
		}
	}
	return FALSE;
}
