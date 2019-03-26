#include "DirectX12EnginePCH.h"
#include "X12Adapter.h"


X12Adapter::X12Adapter()
{
}


X12Adapter::~X12Adapter()
{
}

HRESULT X12Adapter::CreateDevice(IDXGIAdapter1* adapter, const D3D_FEATURE_LEVEL & featureLevel, const UINT & descriptorHeapSize)
{
	if (m_device || m_cpuDescriptorHeap)
		return E_INVALIDARG;

	HRESULT hr = 0;
	if (FAILED(hr = D3D12CreateDevice(
		adapter,
		featureLevel,
		IID_PPV_ARGS(&m_device))))
	{
		return hr;
	}
	
	D3D12_DESCRIPTOR_HEAP_DESC desc{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorHeapSize, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
	if (FAILED(hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_cpuDescriptorHeap))))
	{
		return hr;
	}
	m_incrementalSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return hr;
}

ID3D12Device* X12Adapter::GetDevice() const
{
	return m_device;
}

ID3D12DescriptorHeap* X12Adapter::GetCpuDescriptorHeap() const
{
	return m_cpuDescriptorHeap;
}

X12Adapter::Handle X12Adapter::GetNextHandle()
{
	const SIZE_T offset = m_currentIndex * m_incrementalSize;
	const Handle handle {offset, {m_cpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + offset}};

	m_currentIndex++;
	return handle;
}

ULONG X12Adapter::Release()
{
	SAFE_RELEASE(m_cpuDescriptorHeap);	

	const ULONG ret = m_device ? m_device->Release() : 0;
	m_device = nullptr;
	return ret;
}
