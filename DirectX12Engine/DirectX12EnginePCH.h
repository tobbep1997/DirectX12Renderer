#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi.h>
#include <D3DX12/d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "DXGI.lib")

#include <comdef.h>

#include "Window/Window.h"
#include "DirectX/RenderingManager.h"

#include "DirectX/Shaders/ShaderCreator.h"


#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = nullptr; } }

inline HRESULT SET_NAME(ID3D12Object * object, const std::wstring & name)
{
#ifdef _DEBUG
	return object->SetName(name.c_str());
#endif
	return S_OK;
}