#pragma once
#include "Template/IX12Object.h"

class X12ShaderResourceView :
	public IX12Object
{
public:
	X12ShaderResourceView(RenderingManager * renderingManager, const Window & window);
	~X12ShaderResourceView();

	HRESULT CreateShaderResourceView(const UINT & width = 0, const UINT & height = 0, const UINT& arraySize = 1, const DXGI_FORMAT& format = DXGI_FORMAT_R8G8B8A8_UNORM);

	void CopySubresource(const UINT & dstIndex, ID3D12Resource * resource, ID3D12DescriptorHeap * descriptorHeap) const;

	void SetGraphicsRootDescriptorTable(const UINT & rootParameterIndex);

	ID3D12Resource * GetResource() const;
	ID3D12DescriptorHeap * GetDescriptorHeap() const;

	void Release() override;
private:
	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 1;
	ID3D12Resource * m_resource = nullptr;
	ID3D12DescriptorHeap * m_descriptorHeap = nullptr;
};

