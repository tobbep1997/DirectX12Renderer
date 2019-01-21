#pragma once
#include "../IManagedObject.h"
#include "DirectX/Objects/Mesh/StaticMesh.h"
#include "../Rendering/DXRenderingManager.h"

namespace ID3D12
{

	public ref class DXStaticMesh : IManagedObject<StaticMesh>
	{
	public:
		DXStaticMesh();
		~DXStaticMesh();
		bool Init();
		void Release();

		bool LoadStaticMesh(System::String^ path);
		bool CreateBuffer(DXRenderingManager^ renderingManager);


	};

}