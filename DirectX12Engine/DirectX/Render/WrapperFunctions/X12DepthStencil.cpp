#include "DirectX12EnginePCH.h"
#include "X12DepthStencil.h"
#include <wincodec.h>

X12DepthStencil::X12DepthStencil(RenderingManager* renderingManager, const Window& window, ID3D12GraphicsCommandList * commandList)
	: IX12Object(renderingManager, window, commandList)
{
	m_currentState = {};
}

X12DepthStencil::~X12DepthStencil()
{
}

HRESULT X12DepthStencil::CreateDepthStencil(const std::wstring & name, 
	const UINT & width, const UINT & height,
	const UINT & arraySize,
	const BOOL & createTextureHeap)
{
	HRESULT hr = 0;
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};


	m_width = width;
	m_height = height;
	m_arraySize = arraySize;

	if (width == 0 || height == 0)
	{
		m_width = p_window->GetWidth();
		m_height = p_window->GetHeight();
	}

	dsvHeapDesc.NumDescriptors = arraySize;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	   
	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(&m_depthStencilDescriptorHeap))))
	{
		SET_NAME(m_depthStencilDescriptorHeap, name + L" DepthStencil DescriptorHeap");
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = arraySize ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		if (D3D12_DSV_DIMENSION_TEXTURE2DARRAY == depthStencilDesc.ViewDimension)
		{
			depthStencilDesc.Texture2DArray.ArraySize = arraySize;
			depthStencilDesc.Texture2DArray.FirstArraySlice = 0;
			depthStencilDesc.Texture2DArray.MipSlice = 0;
		}
		
		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);;
		
		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(
				DXGI_FORMAT_D32_FLOAT,
				m_width, m_height,
				arraySize, 1, 1, 0,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencilBuffer))))
		{
			if (createTextureHeap)
			{
				
				D3D12_DESCRIPTOR_HEAP_DESC textureHeapDesc = {};
				textureHeapDesc.NumDescriptors = 1;
				textureHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				textureHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

				if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
					&textureHeapDesc, IID_PPV_ARGS(&m_depthStencilTextureDescriptorHeap))))
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
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
						m_depthStencilBuffer,
						&srvDesc,
						m_depthStencilTextureDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
					);
				}
			}

			SET_NAME(m_depthStencilBuffer, name + L" DepthStencil Resource");
			p_renderingManager->GetDevice()->CreateDepthStencilView(
				m_depthStencilBuffer,
				&depthStencilDesc,
				m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			m_currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		
			SwitchToDSV();
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

ID3D12DescriptorHeap* X12DepthStencil::GetTextureDescriptorHeap() const
{
	return this->m_depthStencilTextureDescriptorHeap;
}

void X12DepthStencil::ClearDepthStencil(ID3D12GraphicsCommandList * commandList) const
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;

	gcl->ClearDepthStencilView(
		m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0,
		nullptr);
	
}

void X12DepthStencil::SwitchToDSV(ID3D12GraphicsCommandList * commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;	
	if (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE == m_currentState)
		gcl->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	m_currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;	
}

void X12DepthStencil::SwitchToSRV(ID3D12GraphicsCommandList * commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;	
	if (D3D12_RESOURCE_STATE_DEPTH_WRITE == m_currentState)
		gcl->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

}


void X12DepthStencil::Release()
{
	SAFE_RELEASE(m_depthStencilBuffer);
	SAFE_RELEASE(m_depthStencilDescriptorHeap);
	SAFE_RELEASE(m_depthStencilTextureDescriptorHeap);
}
