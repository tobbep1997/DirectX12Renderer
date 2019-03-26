#include "DirectX12EnginePCH.h"
#include "X12StructuredBuffer.h"




X12StructuredBuffer::X12StructuredBuffer(RenderingManager* renderingManager, const Window& window,
	ID3D12GraphicsCommandList* commandList) : IX12Object(renderingManager, window, commandList)
{

}

X12StructuredBuffer::~X12StructuredBuffer()
{
}

HRESULT X12StructuredBuffer::Create(const std::wstring & name, const UINT& size)
{
	HRESULT hr = 0;

	const UINT bufferSize = size + 255 & ~255;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_CUSTOM);
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&heapProperties, 
			D3D12_HEAP_FLAG_NONE, 
			&resourceDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
			nullptr,
			IID_PPV_ARGS(&m_resource[i]))))
		{
			this->Release();
			return hr;
		}

		SET_NAME(m_resource[i], name + L" X12StructuredBuffer" + std::to_wstring(i));

		const D3D12_BUFFER_UAV uav {0, 1, bufferSize, 0, D3D12_BUFFER_UAV_FLAG_NONE};
		
		D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
		unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		unorderedAccessViewDesc.Buffer = uav;
			
		m_cpuHandle[i] = 
		{ 
			p_renderingManager->GetCpuDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + 
			p_renderingManager->GetResourceCurrentIndex() * 
			p_renderingManager->GetResourceIncrementalSize()
		};

		p_renderingManager->GetDevice()->CreateUnorderedAccessView(m_resource[i], nullptr, &unorderedAccessViewDesc, m_cpuHandle[i]);

		p_renderingManager->IterateCbvSrvUavDescriptorHeapIndex();

		D3D12_RANGE readRange{ 0,0 };
		if (FAILED(hr = m_resource[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_resourceAddress[i]))))
		{
			return hr;
		}
	}

	return hr;
}

void X12StructuredBuffer::Copy(void* data, const UINT& size, const UINT& offset)
{
	memcpy(m_resourceAddress[p_renderingManager->GetFrameIndex()] + offset, data, size);
}

void X12StructuredBuffer::SetGraphicsRootShaderResourceView(const UINT& rootParameterIndex, ID3D12GraphicsCommandList* commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;
	gcl->SetGraphicsRootShaderResourceView(rootParameterIndex, m_resource[p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress());
}

void X12StructuredBuffer::Release()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_resource[i]);
	}
}
