#include "DXRenderingManager.h"
#include <iostream>

namespace ID3D12
{
	DXRenderingManager::DXRenderingManager()
		: IManagedObject<RenderingManager>(RenderingManager::GetPointerInstance(), true)
	{
	}

	bool DXRenderingManager::Init(DXWindow^ window)
	{
		Window * w = window->GetInstance();
		return SUCCEEDED(p_instance->Init(window->GetInstance(), false));
	}

	void DXRenderingManager::Flush(DXCamera^ camera)
	{
		p_instance->Flush(camera->GetInstance(), 0, TRUE);
	}
	
	void DXRenderingManager::Release()
	{
		p_instance->Present();
	}
}
