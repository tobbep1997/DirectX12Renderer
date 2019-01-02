#pragma once
#include "Template/IX12Object.h"
class X12DepthStencil : public IX12Object
{
public:
	X12DepthStencil(RenderingManager * renderingManager, const Window & window);
	~X12DepthStencil();

	HRESULT CreateDepthStencil(const std::wstring & name, const UINT & width = 0, const UINT & height = 0);
	void Release() override;

	ID3D12Resource * GetResource() const;
	ID3D12DescriptorHeap * GetDescriptorHeap() const;

	void ClearDepthStencil() const;

private:
	ID3D12Resource		* m_depthStencilBuffer = nullptr;
	ID3D12DescriptorHeap* m_depthStencilDescriptorHeap = nullptr;
};

