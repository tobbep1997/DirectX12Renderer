#pragma once
#include "DirectX/Render/WrapperFunctions/Template/IX12Object.h"

class X12Fence : public IX12Object
{
public:
	X12Fence(RenderingManager * renderingManager, const Window & window, ID3D12GraphicsCommandList * commandList = nullptr);
	~X12Fence();

	HRESULT CreateFence(const std::wstring & name, ID3D12CommandQueue * commandQueue);
	HRESULT WaitForCommandList();
	
	void Release() override;
private:
	ID3D12CommandQueue * m_commandQueuePtr = nullptr;

	ID3D12Fence *			m_fence[FRAME_BUFFER_COUNT]{ nullptr };
	HANDLE					m_fenceEvent = nullptr;
	UINT64 					m_fenceValue[FRAME_BUFFER_COUNT]{ 0 };

};

