#include "DirectX12EnginePCH.h"
#include "X12ConstantBuffer.h"





X12ConstantBuffer::X12ConstantBuffer(RenderingManager* renderingManager, const Window& window, ID3D12GraphicsCommandList * commandList)
	: IX12Object(renderingManager, window, commandList)
{
}

X12ConstantBuffer::~X12ConstantBuffer()
{
}

HRESULT X12ConstantBuffer::CreateBuffer(const std::wstring & name, void const* data, const UINT& sizeOf)
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		if (FAILED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_constantBufferDescriptorHeap[i]))))
		{
			return hr;
		}
		SET_NAME(m_constantBufferDescriptorHeap[i], name + L" DescriptorHeap : " + std::to_wstring(i));


		if (FAILED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer[i]))))
		{
			return hr;
		}
		SET_NAME(m_constantBuffer[i], name + L" ConstantBuffer : " + std::to_wstring(i));

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBuffer[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (sizeof(m_constantBuffer) + 255) & ~255;

		p_renderingManager->GetDevice()->CreateConstantBufferView(
			&cbvDesc,
			m_constantBufferDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());

		CD3DX12_RANGE readRange(0, 0);
		if (SUCCEEDED(hr = m_constantBuffer[i]->Map(
			0, &readRange,
			reinterpret_cast<void **>(&m_constantBufferGPUAddress[i]))))
		{
			memcpy(m_constantBufferGPUAddress[i], data, sizeOf);
		}
	}
	return hr;
}

void X12ConstantBuffer::SetComputeRootConstantBufferView(const UINT& rootParameterIndex,
	ID3D12GraphicsCommandList* commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;

	gcl->SetComputeRootConstantBufferView(rootParameterIndex,
		m_constantBuffer[*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress());
}

void X12ConstantBuffer::SetGraphicsRootConstantBufferView(const UINT& rootParameterIndex, ID3D12GraphicsCommandList * commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;

	gcl->SetGraphicsRootConstantBufferView(rootParameterIndex,
		m_constantBuffer[*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress());
}

void X12ConstantBuffer::Copy(void const* data, const UINT& sizeOf)
{
	memcpy(m_constantBufferGPUAddress[*p_renderingManager->GetFrameIndex()], data, sizeOf);
}

void X12ConstantBuffer::Release()
{
	for (UINT j = 0; j < FRAME_BUFFER_COUNT; j++)
	{
		SAFE_RELEASE(m_constantBufferDescriptorHeap[j]);
		SAFE_RELEASE(m_constantBuffer[j]);
	}
}

ID3D12Resource*const* X12ConstantBuffer::GetResource() const
{
	return this->m_constantBuffer;
}

ID3D12DescriptorHeap*const* X12ConstantBuffer::GetDescriptorHeap() const
{
	return this->m_constantBufferDescriptorHeap;
}
