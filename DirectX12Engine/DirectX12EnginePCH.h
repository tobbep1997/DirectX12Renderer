#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi.h>
#include <D3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "DXGI.lib")

#include <comdef.h>

#include "Window/Window.h"
#include "DirectX/RenderingManager.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = nullptr; } }