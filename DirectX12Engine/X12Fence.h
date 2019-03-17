#pragma once
#include "DirectX/Render/WrapperFunctions/Template/IX12Object.h"

class X12Fence : public IX12Object
{
public:
	X12Fence(RenderingManager * renderingManager, const Window & window, ID3D12GraphicsCommandList * commandList = nullptr);
	~X12Fence();

	HRESULT CreateFence(const std::wstring & name);

	HRESULT Signal(ID3D12CommandQueue * commandQueue);
	HRESULT WaitGgu(ID3D12CommandQueue * commandQueue) const;
	HRESULT WaitCpu() const;
	
	void Release() override;
private:

	ID3D12Fence *			m_fence = nullptr;
	HANDLE					m_fenceEvent = nullptr;
	UINT64 					m_fenceValue = 0 ;

};

