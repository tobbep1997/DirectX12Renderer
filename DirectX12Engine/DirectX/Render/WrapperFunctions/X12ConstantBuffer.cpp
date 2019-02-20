#include "DirectX12EnginePCH.h"
#include "X12ConstantBuffer.h"





X12ConstantBuffer::X12ConstantBuffer(RenderingManager* renderingManager, const Window& window, ID3D12GraphicsCommandList * commandList)
	: IX12Object(renderingManager, window, commandList)
{
}

X12ConstantBuffer::~X12ConstantBuffer()
{
}

HRESULT X12ConstantBuffer::CreateBuffer(const std::wstring & name, void const* data, const UINT& sizeOf, const UINT & preAllocData)
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{

		const UINT bufferSize = preAllocData ? preAllocData : 1024 * 64;

		if (FAILED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
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
		m_descriptorHeapOffset = p_renderingManager->GetCbvSrvUavCurrentIndex() * p_renderingManager->GetCbvSrvUavIncrementalSize();
		const D3D12_CPU_DESCRIPTOR_HANDLE handle = 
			{ p_renderingManager->GetCbvSrvUavDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset };
		
		p_renderingManager->GetDevice()->CreateConstantBufferView(
			&cbvDesc,
			handle);

		p_renderingManager->IterateCbvSrvUavDescriptorHeapIndex();

		CD3DX12_RANGE readRange(0, 0);
		if (SUCCEEDED(hr = m_constantBuffer[i]->Map(
			0, &readRange,
			reinterpret_cast<void **>(&m_constantBufferGPUAddress[i]))))
		{
			if (data)
				memcpy(m_constantBufferGPUAddress[i], data, sizeOf);
		}
	}
	return hr;
}

void X12ConstantBuffer::SetComputeRootConstantBufferView(const UINT& rootParameterIndex,
	const UINT & offset, 
	ID3D12GraphicsCommandList* commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;

	gcl->SetComputeRootConstantBufferView(rootParameterIndex,
		m_constantBuffer[*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + offset);
}

void X12ConstantBuffer::SetGraphicsRootConstantBufferView(const UINT& rootParameterIndex, 
	const UINT & offset, 
	ID3D12GraphicsCommandList * commandList)
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : p_commandList;

	gcl->SetGraphicsRootConstantBufferView(rootParameterIndex,
		m_constantBuffer[*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + offset);
}

void X12ConstantBuffer::Copy(void const* data, const UINT& sizeOf, const UINT & offset)
{
	memcpy(m_constantBufferGPUAddress[*p_renderingManager->GetFrameIndex()] + offset, data, sizeOf);
}

void X12ConstantBuffer::Release()
{
	for (UINT j = 0; j < FRAME_BUFFER_COUNT; j++)
	{	
		SAFE_RELEASE(m_constantBuffer[j]);
	}
}

ID3D12Resource*const* X12ConstantBuffer::GetResource() const
{
	return this->m_constantBuffer;
}
