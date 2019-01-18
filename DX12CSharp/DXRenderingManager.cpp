#include "DXRenderingManager.h"

namespace ID3D12
{
	DXRenderingManager::DXRenderingManager()
		: IManagedObject<RenderingManager>(RenderingManager::GetInstance(), false)
	{

	}

	bool DXRenderingManager::Init(DXWindow^ window)
	{
		p_instance->UnsafeInit(window->GetInstance(), false);
		return true;
	}

	void DXRenderingManager::Flush()
	{
		p_instance->Flush(nullptr, 0, TRUE);
	}

	void DXRenderingManager::Release()
	{
		p_instance->Present();
	}
}
