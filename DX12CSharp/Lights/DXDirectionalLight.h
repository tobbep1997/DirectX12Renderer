#pragma once
#include "../Math/DXVector.h"
#include "DXILight.h"
#include "../Rendering/DXRenderingManager.h"

namespace ID3D12
{
	public ref class DXDirectionalLight : public DXILight
	{
	public:
		DXDirectionalLight(DXRenderingManager^ renderingManager, DXWindow^ window);

		bool Init();
		void Update();
		void Release();

		void SetDirection(DXMath::DXVector^ direction);
		
		void SetPosition(DXMath::DXVector^ position);
	};

}
