#include "DirectX12EnginePCH.h"
#include "ParticleEmitter.h"
#include "DirectX/Render/ParticlePass.h"

ParticleEmitter::ParticleEmitter(RenderingManager* renderingManager)
{
	this->m_renderingManager = renderingManager;
}

ParticleEmitter::~ParticleEmitter()
{
}

void ParticleEmitter::Init()
{
	HRESULT hr = 0;	
	if (SUCCEEDED(hr = _createCommandList()))
	{
		if (SUCCEEDED(hr = _createBuffer()))
		{
			
		}
	}
	if (FAILED(hr))
		this->Release();

	m_positions = std::vector<DirectX::XMFLOAT4>();
	m_positions.push_back(DirectX::XMFLOAT4(5, 5, 5, 1));
}

void ParticleEmitter::Release()
{
	SAFE_RELEASE(m_resource);
	SAFE_RELEASE(m_descriptorHeap);

	SAFE_RELEASE(m_commandList);
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_commandAllocator[i]);
	}
}

ID3D12Resource* ParticleEmitter::GetResource() const
{
	return this->m_resource;
}

ID3D12DescriptorHeap* ParticleEmitter::GetDescriptorHeap() const
{
	return this->m_descriptorHeap;
}

ID3D12GraphicsCommandList* ParticleEmitter::GetCommandList() const
{
	return this->m_commandList;
}

const std::vector<DirectX::XMFLOAT4>& ParticleEmitter::GetPositions() const
{
	return this->m_positions;
}

HRESULT ParticleEmitter::OpenCommandList()
{
	HRESULT hr = 0;
	const UINT frameIndex = *m_renderingManager->GetFrameIndex();
	if (SUCCEEDED(hr = this->m_commandAllocator[frameIndex]->Reset()))
	{
		if (SUCCEEDED(hr = this->m_commandList->Reset(this->m_commandAllocator[frameIndex], nullptr)))
		{

		}
	}
	return hr;
}

HRESULT ParticleEmitter::ExecuteCommandList() const
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = m_commandList->Close()))
	{
		ID3D12CommandList* ppCommandLists[] = { m_commandList };
		m_renderingManager->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	return hr;
}

HRESULT ParticleEmitter::_createCommandList()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_renderingManager->GetDevice()->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandAllocator[i]))))
		{
			return hr;
		}
	}
	if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator[0],
		nullptr,
		IID_PPV_ARGS(&m_commandList))))
	{
		m_commandList->Close();
	}
	return hr;
}

HRESULT ParticleEmitter::_createBuffer()
{
	HRESULT hr = 0;

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateDescriptorHeap(
		&descriptorHeapDesc,
		IID_PPV_ARGS(&m_descriptorHeap))))
	{
		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(1024 * 64);
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&m_resource))))
		{
			SET_NAME(m_resource, L"Particle UAV output");

			D3D12_BUFFER_UAV uav{};
			uav.NumElements = 256;
			uav.FirstElement = 0;
			uav.StructureByteStride = sizeof(DirectX::XMFLOAT4);
			uav.CounterOffsetInBytes = 0;
			uav.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
			unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
			unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			unorderedAccessViewDesc.Buffer = uav;

			m_renderingManager->GetDevice()->CreateUnorderedAccessView(
				m_resource,
				nullptr,
				&unorderedAccessViewDesc,
				m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
		}



	}
	
	return hr;
}

void ParticleEmitter::Draw()
{
	m_renderingManager->GetParticlePass()->AddEmitter(this);
}
