#pragma once

#include <chrono>

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
#include "Window/Input.h"
#include "DirectX/Structs.h"
#include "DirectX/RenderingManager.h"

#include "DirectX/Objects/Camera.h"
#include "DirectX/Objects/Drawable.h"
#include "DirectX/Objects/Transform.h"
#include "DirectX/Objects/Light/Template/ILight.h"
#include "DirectX/Objects/Texture/Texture.h"

#include "DirectX/Shaders/ShaderCreator.h"

#include "DirectX/Objects/Light/PointLight.h"
#include "DirectX/Objects/Light/DirectionalLight.h"


inline HRESULT SET_NAME(ID3D12Object * object, const std::wstring & name)
{
#ifdef _DEBUG
	return object->SetName(name.c_str());
#endif
	return S_OK;
}

namespace DEBUG
{
	inline void Print(const std::wstring & w_string)
	{
		OutputDebugStringW(w_string.c_str());
	}

	inline void Print(const std::string & string)
	{
		Print(std::wstring(string.begin(), string.end()));
	}

	inline void Print(const HRESULT & hr)
	{
		const _com_error err(hr);		
		return Print(err.ErrorMessage());
	}

	inline std::wstring StringToWstring(const std::string & string)
	{
		return std::wstring(string.begin(), string.end());
	}

	inline std::string WstringToString(const std::wstring & string)
	{
		return std::string(string.begin(), string.end());
	}
}

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = nullptr; } }
#ifdef _DEBUG
	#include <iostream>
	#define PRINT(p) { DEBUG::Print(p);		std::cout << p;			}
	#define NEW_LINE { DEBUG::Print("\n");	std::cout << std::endl;	}
#else
	#define PRINT(p) {}
	#define NEW_LINE {}
#endif

