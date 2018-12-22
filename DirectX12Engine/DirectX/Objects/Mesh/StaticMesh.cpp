#include "DirectX12EnginePCH.h"
#include "StaticMesh.h"

#include "Utility/Operators.h"


StaticMesh::StaticMesh()
{
	this->Init();
}

StaticMesh::~StaticMesh()
{
	this->Release();
}

void StaticMesh::_clearMesh()
{
	for (UINT i = 0; i < m_staticMesh.size(); i++)
	{
		delete m_staticMesh[i];
	}
	m_staticMesh.clear();
	
}

void StaticMesh::_createMesh(const aiScene* scene)
{
	/*if (!scene->HasMeshes())
		Window::CreateError("Mesh has no Mesh");

	_clearMesh();
	for (UINT i = 0; i < scene->mNumMeshes; i++)
	{
		StaticVertex * vertex = new StaticVertex();
		vertex->Position	= Convert_Assimp_To_DirectX(*scene->mMeshes[i]->mVertices);
		vertex->Normal		= Convert_Assimp_To_DirectX(*scene->mMeshes[i]->mNormals);
		vertex->Tangent		= Convert_Assimp_To_DirectX(*scene->mMeshes[i]->mTangents);
		vertex->TexCord		= Convert_Assimp_To_DirectX(*scene->mMeshes[i]->mTextureCoords[0]);
		m_staticMesh.push_back(vertex);
	}*/
}

void StaticMesh::Init()
{
	m_staticMesh = std::vector<StaticVertex*>();
}

void StaticMesh::Update()
{
}

void StaticMesh::Release()
{
	_clearMesh();
}

void StaticMesh::SetMesh(const std::vector<StaticVertex *>& mesh)
{
	this->m_staticMesh = mesh;
}

void StaticMesh::LoadStaticMesh(const std::string& path)
{
	//Assimp::Importer importer;
	//const aiScene * scene = importer.ReadFile(path.c_str(),
	//	aiProcess_CalcTangentSpace		|
	//	aiProcess_Triangulate			|
	//	aiProcess_JoinIdenticalVertices |
	//	aiProcess_SortByPType);

	//if (!scene)
	//{
	//	Window::CreateError(importer.GetErrorString());
	//}
	//_createMesh(scene);
	
}

BOOL StaticMesh::CreateBuffer(RenderingManager* renderingManager)
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = _createBuffer(renderingManager)))
	{
		if (SUCCEEDED(hr = _signalGPU(renderingManager)))
		{
			return TRUE;
		}
	}
	return FALSE;	
}

const std::vector<StaticVertex *>& StaticMesh::GetStaticMesh() const
{
	return this->m_staticMesh;
}

HRESULT StaticMesh::_createBuffer(RenderingManager* renderingManager)
{
	HRESULT hr = 0;

	m_vertexBufferSize = sizeof(this->m_staticMesh.data());

	if (SUCCEEDED(hr = renderingManager->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer))))
	{
		if (SUCCEEDED(hr = renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexHeapBuffer))))
		{
			SET_NAME(m_vertexHeapBuffer, L"Vertex Buffer Upload Resource Heap");


			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = reinterpret_cast<void*>(this->m_staticMesh.data());
			vertexData.RowPitch = m_vertexBufferSize;
			vertexData.SlicePitch = m_vertexBufferSize;

			UpdateSubresources(renderingManager->GetCommandList(), m_vertexBuffer, m_vertexHeapBuffer, 0, 0, 1, &vertexData);

			renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		}
	}

	return hr;
}

HRESULT StaticMesh::_signalGPU(RenderingManager* renderingManager)
{
	HRESULT hr = 0;

	renderingManager->GetCommandList()->Close();
	ID3D12CommandList* ppCommandLists[] = { renderingManager->GetCommandList() };
	renderingManager->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	renderingManager->GetFenceValues()[*renderingManager->GetFrameIndex()]++;
	if (SUCCEEDED(hr = renderingManager->GetCommandQueue()->Signal(
		&renderingManager->GetFence()[*renderingManager->GetFrameIndex()],
		renderingManager->GetFenceValues()[*renderingManager->GetFrameIndex()])))
	{

		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(StaticVertex);
		m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
	}

	return hr;
}
