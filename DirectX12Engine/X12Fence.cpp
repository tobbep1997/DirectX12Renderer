#include "DirectX12EnginePCH.h"
#include "X12Fence.h"



X12Fence::X12Fence(RenderingManager* renderingManager, const Window& window, ID3D12GraphicsCommandList* commandList) 
	: IX12Object(renderingManager, window, commandList)
{
}

X12Fence::~X12Fence()
{
}

HRESULT X12Fence::CreateFence(const std::wstring & name, ID3D12CommandQueue* commandQueue)
{
	HRESULT hr = 0;

	m_commandQueuePtr = commandQueue;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT && SUCCEEDED(hr); i++)
	{
		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i]))))
		{
			m_fence[i]->SetName(name.c_str());
			m_fenceValue[i] = 0;
		}
	}
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	return hr;
}

HRESULT X12Fence::WaitForCommandList()
{
	const UINT frameIndex = *p_renderingManager->GetFrameIndex();

	HRESULT hr = 0;

	if (SUCCEEDED(hr = m_commandQueuePtr->Signal(m_fence[frameIndex], m_fenceValue[frameIndex])))
	{
		if (m_fence[frameIndex]->GetCompletedValue() < m_fenceValue[frameIndex])
		{
			if (SUCCEEDED(m_fence[frameIndex]->SetEventOnCompletion(m_fenceValue[frameIndex], m_fenceEvent)))
			{
				if (FAILED(hr = m_commandQueuePtr->Wait(m_fence[frameIndex], m_fenceValue[frameIndex])))
				{
					return hr;
				}	
				//WaitForSingleObject(m_fenceEvent, INFINITE);
			}			
		}
		m_fenceValue[frameIndex]++;
	}
	return hr;
}

void X12Fence::Release()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_fence[i]);
	}
}
