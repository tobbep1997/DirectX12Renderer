#pragma once
#include "Template/IX12Object.h"

class X12ShaderResourceView :
	public IX12Object
{
public:
	X12ShaderResourceView() = default;
	~X12ShaderResourceView() = default;

	HRESULT CreateShaderResourceView(
		const UINT & width = 0, 
		const UINT & height = 0, 
		const UINT& arraySize = 1, 
		const DXGI_FORMAT& format = DXGI_FORMAT_R8G8B8A8_UNORM);

	void BeginCopy(ID3D12GraphicsCommandList * commandList) const;
	void EndCopy(ID3D12GraphicsCommandList * commandList) const;
	void CopySubresource(ID3D12GraphicsCommandList * commandList, const UINT & dstIndex, ID3D12Resource * resource) const;
	void CopyDescriptorHeap();

	void SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList * commandList, const UINT & rootParameterIndex);

	ID3D12Resource * GetResource() const;

	const D3D12_CPU_DESCRIPTOR_HANDLE & GetCpuDescriptorHandle() const;

	void Release() override;
private:
	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 1;

	ID3D12Resource * m_resource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{ 0 };
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{ 0 };
};

