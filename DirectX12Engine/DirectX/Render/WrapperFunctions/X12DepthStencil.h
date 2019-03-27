#pragma once
#include "Template/IX12Object.h"
class X12DepthStencil : public IX12Object
{
public:
	X12DepthStencil() = default;
	~X12DepthStencil() = default;

	HRESULT CreateDepthStencil(const std::wstring & name, 
		const UINT & width = 0, const UINT & height = 0,
		const UINT & arraySize = 1,
		const BOOL & createTextureHeap = FALSE);
	void Release() override;

	ID3D12Resource * GetResource() const;
	ID3D12DescriptorHeap * GetDescriptorHeap() const;

	void ClearDepthStencil(ID3D12GraphicsCommandList * commandList) const;

	void SwitchToDSV(ID3D12GraphicsCommandList * commandList);
	void SwitchToSRV(ID3D12GraphicsCommandList * commandList);

	void CopyDescriptorHeap();
	void SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList * commandList, const UINT & rootParameterIndex) const;

	const D3D12_CPU_DESCRIPTOR_HANDLE & GetCpuDescriptorHeap() const;

private:
	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 1;

	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{0};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{0};
	ID3D12Resource		* m_depthStencilBuffer = nullptr;
	ID3D12DescriptorHeap* m_depthStencilDescriptorHeap = nullptr;	
	
	D3D12_RESOURCE_STATES m_currentState {};
};

