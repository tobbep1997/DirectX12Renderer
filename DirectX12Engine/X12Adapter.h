#pragma once

class X12Adapter
{
struct Handle
{
	SIZE_T Offset;
	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
};

public:
	X12Adapter();
	~X12Adapter();

	HRESULT CreateDevice(IDXGIAdapter1* adapter, 
		const D3D_FEATURE_LEVEL& featureLevel = D3D_FEATURE_LEVEL_12_0, 
		const UINT& descriptorHeapSize = MAX_DESCRIPTOR_SIZE);

	ID3D12Device * GetDevice() const;
	ID3D12DescriptorHeap * GetCpuDescriptorHeap() const;

	Handle GetNextHandle();
	
	ULONG Release();

private:
	ID3D12Device * m_device = nullptr;
	ID3D12DescriptorHeap * m_cpuDescriptorHeap = nullptr;

	SIZE_T m_currentIndex = 0;
	SIZE_T m_incrementalSize = 0;
	
};

