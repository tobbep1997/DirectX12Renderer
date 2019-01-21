#pragma once
#include "IManagedObject.h"
#include "DirectX/Objects/Texture/Texture.h"
#include "Rendering/DXRenderingManager.h"

namespace ID3D12
{

	public ref class DXTexture : IManagedObject<Texture>
	{
	public:
		DXTexture();
		void Release();
		void SetRenderingManager(DXRenderingManager renderingManager);

		bool LoadTexture(System::String^ path);
		bool LoadTexture(System::String^ path, bool generateMip);
		bool LoadTexture(System::String^ path, bool generateMip, DXRenderingManager^ renderingManager);
		bool LoadDDSTexture(System::String^ path);
		bool LoadDDSTexture(System::String^ path, bool generateMip);
		bool LoadDDSTexture(System::String^ path, bool generateMip, DXRenderingManager^ renderingManager);
	};

}
