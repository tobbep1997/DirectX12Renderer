#include "DirectX12EnginePCH.h"
#include "StaticMesh.h"

#include "Utility/Operators.h"

#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>

StaticMesh::StaticMesh()
{
	m_renderingManager = RenderingManager::GetInstance();
	SAFE_NEW(m_fence, new X12Fence());
}

StaticMesh::~StaticMesh()
{
	SAFE_DELETE(m_fence);
}

void StaticMesh::_clearMesh()
{
	m_staticMesh.clear();
}

BOOL StaticMesh::_createMesh(const aiScene* scene)
{
	if (!scene->HasMeshes())
		return FALSE;

	_clearMesh();
	for (UINT i = 0; i < scene->mNumMeshes; i++)
	{
		for (UINT j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
		{
			StaticVertex vertex = {};
			vertex.Position		= Convert_Assimp_To_DirectX(scene->mMeshes[i]->mVertices[j]);
			vertex.Normal		= Convert_Assimp_To_DirectX(scene->mMeshes[i]->mNormals[j], 0);
			vertex.Tangent		= Convert_Assimp_To_DirectX(scene->mMeshes[i]->mTangents[j], 0);
			vertex.TexCord		= Convert_Assimp_To_DirectX(scene->mMeshes[i]->mTextureCoords[0][j], 0);
			m_staticMesh.push_back(vertex);
		}
	}
	return TRUE;
}

BOOL StaticMesh::Init()
{
	m_staticMesh = std::vector<StaticVertex>();
	m_fence->CreateFence(L"Mesh", m_renderingManager->GetMainAdapter()->GetDevice());
	return TRUE;
}

void StaticMesh::Update()
{
}

void StaticMesh::Release()
{
	_clearMesh();
	if (m_fence)
		m_fence->Release();
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_vertexHeapBuffer);
}

const BOOL & StaticMesh::LoadStaticMesh(const std::string& path)
{
	this->m_name = path;
	Assimp::Importer importer;
	const aiScene * scene = importer.ReadFile(path.c_str(),
		aiProcess_CalcTangentSpace		|
		aiProcess_Triangulate			|
		aiProcess_SortByPType			|
		aiProcess_FlipUVs);

	if (!scene)
	{
		m_meshLoaded = FALSE;
		return m_meshLoaded;
	}
	m_meshLoaded = _createMesh(scene);
	return m_meshLoaded;
}

BOOL StaticMesh::CreateBuffer()
{
	if (!m_renderingManager)
		m_renderingManager = RenderingManager::GetInstance();

	HRESULT hr = 0;
	if (!m_meshLoaded)
		return FALSE;
	if (SUCCEEDED(hr = m_renderingManager->OpenCommandList()))
	{
		if (SUCCEEDED(hr = _createBuffer()))
		{
			if (SUCCEEDED(hr = m_renderingManager->SignalGPU()))
			{
				m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
				m_vertexBufferView.StrideInBytes = sizeof(StaticVertex);
				m_vertexBufferView.SizeInBytes = m_vertexBufferSize;

				if (SUCCEEDED(hr = m_fence->Signal(m_renderingManager->GetCommandQueue())))
				{
					if (SUCCEEDED(hr = m_fence->WaitCpu()))
					{
						
					}
					
				}

				return TRUE;
			}			
		}
	}
	return FALSE;	
}

const std::vector<StaticVertex>& StaticMesh::GetStaticMesh() const
{
	return this->m_staticMesh;
}

const D3D12_VERTEX_BUFFER_VIEW& StaticMesh::GetVertexBufferView() const
{
	return this->m_vertexBufferView;
}

HRESULT StaticMesh::_createBuffer()
{
	HRESULT hr = 0;
	m_vertexBufferSize = static_cast<UINT>(sizeof(StaticVertex) * this->m_staticMesh.size());

	if (SUCCEEDED(hr = m_renderingManager->GetMainAdapter()->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer))))
	{
		SET_NAME(m_vertexBuffer, std::wstring(L"StaticMesh :") +
			std::wstring(m_name.begin(), m_name.end()) +
			std::wstring(L": vertexBuffer"));

		if (SUCCEEDED(hr = m_renderingManager->GetMainAdapter()->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexHeapBuffer))))
		{
			SET_NAME(m_vertexHeapBuffer, std::wstring(L"StaticMesh :") +
				std::wstring(m_name.begin(), m_name.end()) +
				std::wstring(L": vertexHeapBuffer"));

			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = reinterpret_cast<void*>(this->m_staticMesh.data());
			vertexData.RowPitch = m_vertexBufferSize;
			vertexData.SlicePitch = m_vertexBufferSize;

			UpdateSubresources(m_renderingManager->GetCommandList(), m_vertexBuffer, m_vertexHeapBuffer, 0, 0, 1, &vertexData);

			m_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		}
	}

	return hr;
}
