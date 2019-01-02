#include "DirectX12EnginePCH.h"
#include "X12DepthStencil.h"
#include <wincodec.h>

X12DepthStencil::X12DepthStencil(RenderingManager* renderingManager, const Window& window)
	: IX12Object(renderingManager, window)
{
	m_currentState = {};
}

X12DepthStencil::~X12DepthStencil()
{
}

HRESULT X12DepthStencil::CreateDepthStencil(const std::wstring & name, 
	const UINT & width, const UINT & height,
	const BOOL & createTextureHeap)
{
	HRESULT hr = 0;
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};

	UINT w = width, h = height;

	if (width == 0 || height == 0)
	{
		w = p_window->GetWidth();
		h = p_window->GetHeight();
	}

	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	   
	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(&m_depthStencilDescriptorHeap))))
	{
		SET_NAME(m_depthStencilDescriptorHeap, name + L" DepthStencil DescriptorHeap");
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(
				DXGI_FORMAT_D32_FLOAT,
				w, h,
				1, 0, 1, 0,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencilBuffer))))
		{
			SET_NAME(m_depthStencilBuffer, name + L" DepthStencil Resource");
			p_renderingManager->GetDevice()->CreateDepthStencilView(
				m_depthStencilBuffer,
				&depthStencilDesc,
				m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			m_currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			if (createTextureHeap)
			{
				D3D12_DESCRIPTOR_HEAP_DESC textureHeapDesc = {};
				textureHeapDesc.NumDescriptors = 1;
				textureHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				textureHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

				SwitchToSRV();

				if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
					&textureHeapDesc, IID_PPV_ARGS(&m_depthStencilTextureDescriptorHeap))))
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					srvDesc.Texture2D.MipLevels = 1;

					p_renderingManager->GetDevice()->CreateShaderResourceView(
						m_depthStencilBuffer, 
						&srvDesc,
						m_depthStencilTextureDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
					);
				}			
			}
		}
	}

	return hr;
}

ID3D12Resource* X12DepthStencil::GetResource() const
{
	return this->m_depthStencilBuffer;
}

ID3D12DescriptorHeap* X12DepthStencil::GetDescriptorHeap() const
{
	return this->m_depthStencilDescriptorHeap;
}

ID3D12Resource* X12DepthStencil::GetTextureResource() const
{
	return this->m_depthTextureUploadHeap;
}

ID3D12DescriptorHeap* X12DepthStencil::GetTextureDescriptorHeap() const
{
	return this->m_depthStencilTextureDescriptorHeap;
}

void X12DepthStencil::ClearDepthStencil() const
{
	p_renderingManager->GetCommandList()->ClearDepthStencilView(
		m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0,
		nullptr);
}

void X12DepthStencil::SwitchToDSV()
{
	if (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE == m_currentState)
		p_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	m_currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
}

void X12DepthStencil::SwitchToSRV()
{
	if (D3D12_RESOURCE_STATE_DEPTH_WRITE == m_currentState)
		p_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}


void X12DepthStencil::Release()
{
	SAFE_RELEASE(m_depthStencilBuffer);
	SAFE_RELEASE(m_depthStencilDescriptorHeap);

	SAFE_RELEASE(m_depthTextureUploadHeap);
	SAFE_RELEASE(m_depthStencilTextureDescriptorHeap);
}
