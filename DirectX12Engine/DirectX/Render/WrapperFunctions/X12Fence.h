#pragma once
#include "Template/IX12Object.h"

class X12Fence : public IX12Object
{
public:
	X12Fence() = default;
	~X12Fence() = default;

	HRESULT CreateFence(const std::wstring & name, ID3D12Device * device, const D3D12_FENCE_FLAGS & flag = D3D12_FENCE_FLAG_NONE);

	HRESULT Signal(ID3D12CommandQueue * commandQueue);
	HRESULT WaitGgu(ID3D12CommandQueue * commandQueue) const;
	HRESULT WaitCpu() const;

	const bool IsDone() const;

	void Release() override;
private:

	ID3D12Fence *			m_fence = nullptr;
	HANDLE					m_fenceEvent = nullptr;
	UINT64 					m_fenceValue = 0 ;

};

