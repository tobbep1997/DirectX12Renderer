#include "DirectX12EnginePCH.h"
#include "ReflectionPass.h"


ReflectionPass::ReflectionPass(RenderingManager * renderingManager, const Window & window) : IRender(renderingManager, window)
{
}


ReflectionPass::~ReflectionPass()
{
}

HRESULT ReflectionPass::Init()
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = _preInit()))
	{
		if (SUCCEEDED(hr = p_renderingManager->SignalGPU(p_commandList[*p_renderingManager->GetFrameIndex()])))
		{
			
		}
	}
	if (FAILED(hr))
	{
		this->Release();
	}
	return hr;
}

void ReflectionPass::Update(const Camera& camera, const float& deltaTime)
{
}

void ReflectionPass::Draw()
{
}

void ReflectionPass::Clear()
{
}

void ReflectionPass::Release()
{
}

HRESULT ReflectionPass::_preInit()
{
	HRESULT hr = 0;
	if (FAILED(hr = p_createCommandList(L"Reflection")))
	{
		return hr;
	}

	

	return hr;
}
