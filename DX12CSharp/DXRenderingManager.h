#pragma once
#include "DXWindow.h"
#include <DirectX/RenderingManager.h>

namespace ID3D12
{
	using namespace System;
	public ref class DXRenderingManager : IManagedObject<RenderingManager>
	{
	public:
		DXRenderingManager();

		bool Init(DXWindow^ window);
		void Flush();
		void Release();
	};

}
