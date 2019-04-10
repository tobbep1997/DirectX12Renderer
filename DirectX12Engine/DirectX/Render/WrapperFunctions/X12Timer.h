#pragma once
#include "Template/IX12Object.h"


class X12Timer :
	public IX12Object
{
public:

	struct TimeStamp
	{
		UINT64 Start = 0;
		UINT64 Stop = 0;
	};

	X12Timer();
	~X12Timer();

	HRESULT CreateTimer(const UINT & numberOfIndexes, ID3D12Device * device = nullptr, const D3D12_QUERY_HEAP_TYPE & heapType = D3D12_QUERY_HEAP_TYPE_TIMESTAMP);
	void Release() override;

	void SetCommandQueue(ID3D12CommandQueue * commandQueue);
	ID3D12CommandQueue * GetCommandQueue() const;

	void Start(ID3D12GraphicsCommandList * commandList);
	void Stop(ID3D12GraphicsCommandList * commandList);
	void ResolveQueryToCpu(ID3D12GraphicsCommandList* commandList) const;
	void CountTimer();

	TimeStamp GetTimeStamp(const UINT & index) const;

	const UINT & GetCount() const;

	void PrintToFile(const char * path) const;
	void ExportToMagnusTimelineCreator(const char * path) const;

private:
	void _start(ID3D12GraphicsCommandList * commandList, const UINT & timeStampIndex);
	void _stop(ID3D12GraphicsCommandList * commandList, const UINT & timeStampIndex);

private:
	UINT m_counter = 0;
	ID3D12CommandQueue * m_pCommandQueue = nullptr;

	ID3D12Device*		m_pDevice = nullptr;
	ID3D12QueryHeap*	m_queryHeap = nullptr;
	ID3D12Resource*		m_queryResourceCPU = nullptr;
	ID3D12Resource*		m_queryResourceGPU = nullptr;

	bool	m_active = false;
	UINT	m_timerCount = 0;

};

