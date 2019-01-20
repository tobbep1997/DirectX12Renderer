#include "DXStaticMesh.h"
#include "../Converter.h"


namespace ID3D12
{
	using namespace Converter;
	DXStaticMesh::DXStaticMesh() : IManagedObject(new StaticMesh(), true)
	{
	}

	DXStaticMesh::~DXStaticMesh()
	{
		p_instance->Release();
	}

	bool DXStaticMesh::Init()
	{
		return p_instance->Init();
	}

	void DXStaticMesh::Release()
	{
		p_instance->Release();
	}

	bool DXStaticMesh::LoadStaticMesh(System::String^ path)
	{
		return p_instance->LoadStaticMesh(std::string(SystemStringToCharArr(path)));
	}

	bool DXStaticMesh::CreateBuffer(DXRenderingManager^ renderingManager)
	{
		return p_instance->CreateBuffer(renderingManager->GetInstance());
	}

}
