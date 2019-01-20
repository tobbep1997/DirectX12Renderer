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
		return SUCCEEDED(p_instance->Init(window->GetInstance(), true));
	}

	void DXRenderingManager::Flush(DXCamera^ camera)
	{
		p_instance->Flush(camera->GetInstance<Camera>(), 0, TRUE);
	}
	
	void DXRenderingManager::Release()
	{
		p_instance->Release();
	}
}
