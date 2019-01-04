#include "DirectX12EnginePCH.h"
#include "X12RenderTargetView.h"

X12RenderTargetView::X12RenderTargetView(
	RenderingManager* renderingManager, 
	const Window& window)
	: IX12Object(renderingManager, window)
{
}

X12RenderTargetView::~X12RenderTargetView()
{
}

HRESULT X12RenderTargetView::CreateRenderTarget(const UINT& width, const UINT& height)
{
	HRESULT hr = 0;

	m_width = width;
	m_height = height;

	if (m_width == 0 || m_height == 0)
	{
		m_width = p_window->GetWidth();
		m_height = p_window->GetHeight();
	}

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Alignment = 0;
	resourceDesc.DepthOrArraySize = FRAME_BUFFER_COUNT;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Width = m_width;
	resourceDesc.Height = m_height;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = p_renderingManager->GetDevice()->GetResourceAllocationInfo(0, 1, &resourceDesc);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12Heap * heap = nullptr;
	
	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&m_rtvDescriptorHeap))))
	{
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_HEAP_DESC heapDesc{};
		heapDesc.Alignment = allocationInfo.Alignment;
		heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		heapDesc.Properties = heapProperties;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		depthOptimizedClearValue.Color[0] = m_clearColor[0];
		depthOptimizedClearValue.Color[1] = m_clearColor[1];
		depthOptimizedClearValue.Color[2] = m_clearColor[2];
		depthOptimizedClearValue.Color[3] = m_clearColor[3];

		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateHeap(
			&heapDesc,
			IID_PPV_ARGS(&heap))))
		{
			m_rtvDescriptorSize = p_renderingManager->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

			for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
			{
				if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreatePlacedResource(
					heap,
					0,
					&CD3DX12_RESOURCE_DESC::Tex2D(
						DXGI_FORMAT_R8G8B8A8_UNORM,
						m_width,
						m_height,
						1, 0, 1, 0,
						D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					&depthOptimizedClearValue,
					IID_PPV_ARGS(&m_renderTargets[i]))))
				{
					p_renderingManager->GetDevice()->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);
					rtvHandle.Offset(1, m_rtvDescriptorSize);
				}
			}
		}
	}
	SAFE_RELEASE(heap);

	return hr;
}

ID3D12Resource* const* X12RenderTargetView::GetResource() const
{
	return this->m_renderTargets;
}

ID3D12DescriptorHeap* X12RenderTargetView::GetDescriptorHeap() const
{
	return this->m_rtvDescriptorHeap;
}

const UINT& X12RenderTargetView::GetDescriptorSize() const
{
	return this->m_rtvDescriptorSize;
}

void X12RenderTargetView::Clear(const CD3DX12_CPU_DESCRIPTOR_HANDLE & rtvHandle) const
{
	//const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
	//	m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	//	*p_renderingManager->GetFrameIndex(),
	//	m_rtvDescriptorSize);
	
	p_renderingManager->GetCommandList()->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
}

void X12RenderTargetView::Release()
{
	SAFE_RELEASE(m_rtvDescriptorHeap);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_renderTargets[i]);
	}
}