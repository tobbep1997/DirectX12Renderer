#include "DirectX12EnginePCH.h"
#include "X12Fence.h"

HRESULT X12Fence::CreateFence(const std::wstring& name, ID3D12Device* device, const D3D12_FENCE_FLAGS& flag)
{
	HRESULT hr = 0;


	if (SUCCEEDED(hr = device->CreateFence(
		0, 
		flag, 
		IID_PPV_ARGS(&m_fence))))
	{
		m_fence->SetName(name.c_str());
		m_fenceValue = 0;
	}

	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	return hr;
}

HRESULT X12Fence::Signal(ID3D12CommandQueue* commandQueue)
{
	m_fenceValue++;
	return commandQueue->Signal(m_fence, m_fenceValue);
}

HRESULT X12Fence::WaitGgu(ID3D12CommandQueue * commandQueue) const
{
	HRESULT hr = 0;

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		if (FAILED(hr = commandQueue->Wait(m_fence, m_fenceValue)))
		{
			return hr;
		}
	}

	return hr;
}

HRESULT X12Fence::WaitCpu() const
{
	HRESULT hr = 0;

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		if (FAILED(hr = m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
		{
			return hr;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	return hr;
}



void X12Fence::Release()
{
	SAFE_RELEASE(m_fence);
}
