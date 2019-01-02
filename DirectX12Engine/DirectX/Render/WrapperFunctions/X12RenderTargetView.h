#pragma once
#include "Template/IX12Object.h"

class X12RenderTargetView :
	public IX12Object
{
public:
	X12RenderTargetView(RenderingManager * renderingManager, const Window & window);
	~X12RenderTargetView();

	HRESULT CreateRenderTarget(const UINT & width = 0, const UINT & height = 0);

	ID3D12Resource *const* GetResource() const;
	ID3D12DescriptorHeap * GetDescriptorHeap() const;
	const UINT & GetDescriptorSize() const;


	void Clear(const CD3DX12_CPU_DESCRIPTOR_HANDLE & rtvHandle) const;
	void Release() override;

private:

	const float m_clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };

	UINT m_width = 0;
	UINT m_height = 0;

	UINT m_rtvDescriptorSize = 0;
	ID3D12DescriptorHeap *	m_rtvDescriptorHeap = nullptr;
	ID3D12Resource *		m_renderTargets[FRAME_BUFFER_COUNT]{ nullptr };
};

