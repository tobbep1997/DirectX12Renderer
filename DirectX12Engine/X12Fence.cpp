#include "DirectX12EnginePCH.h"
#include "X12Fence.h"



X12Fence::X12Fence(RenderingManager* renderingManager, const Window& window, ID3D12GraphicsCommandList* commandList) 
	: IX12Object(renderingManager, window, commandList)
{
}

X12Fence::~X12Fence()
{
}

HRESULT X12Fence::CreateFence(const std::wstring & name)
{
	HRESULT hr = 0;


	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
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