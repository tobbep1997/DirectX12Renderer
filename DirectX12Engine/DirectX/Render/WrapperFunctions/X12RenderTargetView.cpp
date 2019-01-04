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

HRESULT X12RenderTargetView::CreateRenderTarget(const UINT& width, const UINT& height, 
	const UINT & arraySize, const BOOL & createTexture,
	const DXGI_FORMAT & format)
{
	HRESULT hr = 0;

	m_width = width;
	m_height = height;
	m_arraySize = arraySize;

	if (m_width == 0 || m_height == 0)
	{
		m_width = p_window->GetWidth();
		m_height = p_window->GetHeight();
	}

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Alignment = 0;
	resourceDesc.DepthOrArraySize = FRAME_BUFFER_COUNT * arraySize;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Width = m_width;
	resourceDesc.Height = m_height;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = p_renderingManager->GetDevice()->GetResourceAllocationInfo(0, 1, &resourceDesc);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT * arraySize;
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
		depthOptimizedClearValue.Format = format;
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

			if (createTexture)
			{
				D3D12_DESCRIPTOR_HEAP_DESC textureHeapDesc = {};
				textureHeapDesc.NumDescriptors = arraySize;
				textureHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				textureHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

				if (FAILED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
					&textureHeapDesc, IID_PPV_ARGS(&m_rtvTextureDescriptorHeap))))
				{
					return hr;
				}
			}

			for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
			{
				if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreatePlacedResource(
					heap,
					0,
					&CD3DX12_RESOURCE_DESC::Tex2D(
						format,
						m_width,
						m_height,
						arraySize, 0, 1, 0,
						D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					&depthOptimizedClearValue,
					IID_PPV_ARGS(&m_renderTargets[i]))))
				{
					D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
					renderTargetViewDesc.Format = format;
					renderTargetViewDesc.ViewDimension = arraySize ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
					if (renderTargetViewDesc.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2D)
					{
						renderTargetViewDesc.Texture2D.MipSlice = 0;
						renderTargetViewDesc.Texture2D.PlaneSlice = 0;
					}
					else
					{
						renderTargetViewDesc.Texture2DArray.ArraySize = arraySize;
						renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
						renderTargetViewDesc.Texture2DArray.MipSlice = 0;
						renderTargetViewDesc.Texture2DArray.PlaneSlice = 0;
					}

					
					p_renderingManager->GetDevice()->CreateRenderTargetView(m_renderTargets[i], &renderTargetViewDesc, rtvHandle);
					rtvHandle.Offset(1, m_rtvDescriptorSize);
					m_currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
					if (createTexture)
					{
						D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
						srvDesc.Format = format;
						srvDesc.ViewDimension = arraySize ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						if (srvDesc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
						{
							srvDesc.Texture2D.MipLevels = 1;
						}
						else
						{
							srvDesc.Texture2DArray.ArraySize = arraySize;
							srvDesc.Texture2DArray.FirstArraySlice = 0;
							srvDesc.Texture2DArray.MipLevels = 1;
							srvDesc.Texture2DArray.MostDetailedMip = 0;							
						}

						p_renderingManager->GetDevice()->CreateShaderResourceView(
							m_renderTargets[i],
							&srvDesc,
							m_rtvTextureDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
						);
					}

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

ID3D12DescriptorHeap* X12RenderTargetView::GetTextureDescriptorHeap() const
{
	return this->m_rtvTextureDescriptorHeap;
}

const UINT& X12RenderTargetView::GetDescriptorSize() const
{
	return this->m_rtvDescriptorSize;
}

void X12RenderTargetView::SwitchToRTV()
{
	if (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE == m_currentState)
		p_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[*p_renderingManager->GetFrameIndex()], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

void X12RenderTargetView::SwitchToSRV()
{
	if (D3D12_RESOURCE_STATE_RENDER_TARGET == m_currentState)
		p_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[*p_renderingManager->GetFrameIndex()], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

void X12RenderTargetView::Clear(const CD3DX12_CPU_DESCRIPTOR_HANDLE & rtvHandle) const
{	
	p_renderingManager->GetCommandList()->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
}

void X12RenderTargetView::Release()
{
	SAFE_RELEASE(m_rtvDescriptorHeap);
	SAFE_RELEASE(m_rtvTextureDescriptorHeap);
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_renderTargets[i]);
	}
}
