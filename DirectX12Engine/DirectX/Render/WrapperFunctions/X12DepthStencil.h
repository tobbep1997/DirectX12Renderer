#pragma once
#include "Template/IX12Object.h"
class X12DepthStencil : public IX12Object
{
public:
	X12DepthStencil(RenderingManager * renderingManager, const Window & window, ID3D12GraphicsCommandList * commandList = nullptr);
	~X12DepthStencil();

	HRESULT CreateDepthStencil(const std::wstring & name, 
		const UINT & width = 0, const UINT & height = 0,
		const UINT & arraySize = 1,
		const BOOL & createTextureHeap = FALSE);
	void Release() override;

	ID3D12Resource * GetResource() const;
	ID3D12DescriptorHeap * GetDescriptorHeap() const;
	ID3D12DescriptorHeap * GetTextureDescriptorHeap() const;

	void ClearDepthStencil(ID3D12GraphicsCommandList * commandList = nullptr) const;

	void SwitchToDSV(ID3D12GraphicsCommandList * commandList = nullptr);
	void SwitchToSRV(ID3D12GraphicsCommandList * commandList = nullptr);

private:
	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 1;

	ID3D12Resource		* m_depthStencilBuffer = nullptr;
	ID3D12DescriptorHeap* m_depthStencilDescriptorHeap = nullptr;
	
	ID3D12DescriptorHeap* m_depthStencilTextureDescriptorHeap = nullptr;

	D3D12_RESOURCE_STATES m_currentState;
};

