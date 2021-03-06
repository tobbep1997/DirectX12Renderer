#include "DirectX12EnginePCH.h"
#include "X12ConstantBuffer.h"


HRESULT X12ConstantBuffer::CreateBuffer(const std::wstring & name, void const* data, const UINT& sizeOf, const UINT & preAllocData)
{
	HRESULT hr = 0;

	const UINT bufferSize = preAllocData ? preAllocData : 1024 * 64;
	const D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	const D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	   
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = p_renderingManager->GetMainAdapter()->GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
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
		
		
		
		m_handle[i] = p_renderingManager->GetMainAdapter()->GetNextHandle().DescriptorHandle;
		p_renderingManager->GetMainAdapter()->GetDevice()->CreateConstantBufferView(
			&cbvDesc,
			m_handle[i]);
		

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

HRESULT X12ConstantBuffer::CreateSharedBuffer(const std::wstring& name, const UINT& sizeOf,
	const UINT& preAllocData)
{
	HRESULT hr = 0;

	const UINT bufferSize = preAllocData ? preAllocData : 1024 * 64;
	const D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	const D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	

	ID3D12Device * device = p_renderingManager->GetSecondAdapter() ? p_renderingManager->GetSecondAdapter()->GetDevice() : nullptr;
	if (!device)
		return E_FAIL;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{

		if (FAILED(hr = device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
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
		


		m_handle[i] = p_renderingManager->GetSecondAdapter()->GetNextHandle().DescriptorHandle;
		device->CreateConstantBufferView(
			&cbvDesc,
			m_handle[i]);

		CD3DX12_RANGE readRange(0, 0);
		if (FAILED(hr = m_constantBuffer[i]->Map(
			0, &readRange,
			reinterpret_cast<void **>(&m_constantBufferGPUAddress[i]))))
		{
			return hr;
		}
	}
	return hr;
}

void X12ConstantBuffer::SetComputeRootConstantBufferView(ID3D12GraphicsCommandList* commandList,
	const UINT& rootParameterIndex,
	const UINT & offset)
{
	
	commandList->SetComputeRootConstantBufferView(rootParameterIndex,
		m_constantBuffer[p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + offset);
}

void X12ConstantBuffer::SetComputeRootShaderResourceView(ID3D12GraphicsCommandList* commandList,
	const UINT& rootParameterIndex,
	const UINT & offset)
{
	commandList->SetComputeRootShaderResourceView(rootParameterIndex,
		m_constantBuffer[p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + offset);
}

void X12ConstantBuffer::SetGraphicsRootConstantBufferView(ID3D12GraphicsCommandList* commandList,
	const UINT& rootParameterIndex,
	const UINT & offset)
{
	commandList->SetGraphicsRootConstantBufferView(rootParameterIndex,
		m_constantBuffer[p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + offset);
}

void X12ConstantBuffer::SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* commandList,
	const UINT& rootParameterIndex,
	const UINT & offset)
{
	commandList->SetGraphicsRootShaderResourceView(rootParameterIndex,
		m_constantBuffer[p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + offset);
}

void X12ConstantBuffer::Copy(void const* data, const UINT& sizeOf, const UINT & offset)
{
	memcpy(m_constantBufferGPUAddress[p_renderingManager->GetFrameIndex()] + offset, data, sizeOf);
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
