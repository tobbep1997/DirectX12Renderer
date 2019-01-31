#include "DirectX12EnginePCH.h"
#include "X12ShaderResourceView.h"
#include "Functions/DXGIFunctions.h"


X12ShaderResourceView::X12ShaderResourceView(RenderingManager* renderingManager, const Window& window, ID3D12GraphicsCommandList * commandList)
	: IX12Object(renderingManager, window, commandList)
{
}

X12ShaderResourceView::~X12ShaderResourceView()
{
}

HRESULT X12ShaderResourceView::CreateShaderResourceView(const UINT& width, const UINT& height, const UINT& arraySize, const DXGI_FORMAT& format)
{
	HRESULT hr = 0;

	m_width = width;
	m_height = height;
	m_arraySize = arraySize;

	if (width == 0 || height == 0)
	{
		m_width = p_window->GetWidth();
		m_height = p_window->GetHeight();
	}


	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		m_width, m_height,
		arraySize, 1, 1, 0,
		D3D12_RESOURCE_FLAG_NONE);
	
	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&m_resource))))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = arraySize - 1 ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D;
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

		m_descriptorHeapOffset = p_renderingManager->GetCbvSrvUavCurrentIndex() * p_renderingManager->GetCbvSrvUavIncrementalSize();
		const D3D12_CPU_DESCRIPTOR_HANDLE handle =
		{ p_renderingManager->GetCbvSrvUavDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset };

		p_renderingManager->GetDevice()->CreateShaderResourceView(
			m_resource,
			&srvDesc,
			handle);

		p_renderingManager->IterateCbvSrvUavDescriptorHeapIndex();

	}
	

	return hr;
}

void X12ShaderResourceView::BeginCopy(ID3D12GraphicsCommandList * commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;
	gcl->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
}

void X12ShaderResourceView::EndCopy(ID3D12GraphicsCommandList * commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;
	gcl->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void X12ShaderResourceView::CopySubresource(const UINT & dstIndex, ID3D12Resource* resource, ID3D12GraphicsCommandList * commandList) const
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;

	gcl->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));

	
	UINT counter = 0;
	const D3D12_RESOURCE_DESC desc = resource->GetDesc();
	for (UINT i = dstIndex; i < desc.DepthOrArraySize + dstIndex; i++)
	{
		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = m_resource;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = i;

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = resource;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.SubresourceIndex = counter++;
		
		gcl->CopyTextureRegion(
			&dstLocation, 
			0, 
			0, 
			0, 
			&srcLocation, 
			nullptr);
	}
	
	gcl->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

}

void X12ShaderResourceView::SetGraphicsRootDescriptorTable(const UINT& rootParameterIndex, ID3D12GraphicsCommandList * commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;

	D3D12_GPU_DESCRIPTOR_HANDLE handle = p_renderingManager->GetCbvSrvUavDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += m_descriptorHeapOffset;

	gcl->SetGraphicsRootDescriptorTable(rootParameterIndex, handle);
}


ID3D12Resource* X12ShaderResourceView::GetResource() const
{
	return m_resource;
}

void X12ShaderResourceView::Release()
{
	SAFE_RELEASE(m_resource);
}
