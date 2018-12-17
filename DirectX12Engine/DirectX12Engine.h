#pragma once
#include "DirectX12EnginePCH.h"

inline HRESULT InitDirectX12Engine(Window * window,
	RenderingManager * renderingManager,
	HINSTANCE hInstance,
	const std::string & windowName,
	const UINT & width,
	const UINT & height,
	const BOOL & fullscreen = FALSE)
{
	HRESULT hr;
	if (window || renderingManager)
		return E_INVALIDARG;

	window = Window::GetInstance();
	renderingManager = RenderingManager::GetInstance();

	if (SUCCEEDED(hr = window->Create(hInstance, windowName, width, height, fullscreen)))
	{
		if (SUCCEEDED(hr = renderingManager->Init(*window)))
		{
			return S_OK;
		}
	}
	return hr;
}
