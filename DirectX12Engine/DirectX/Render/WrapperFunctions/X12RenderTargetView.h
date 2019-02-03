#pragma once
#include "Template/IX12Object.h"

class X12RenderTargetView :
	public IX12Object
{
public:
	X12RenderTargetView(RenderingManager * renderingManager, const Window & window, ID3D12GraphicsCommandList * commandList = nullptr);
	~X12RenderTargetView();

	HRESULT CreateRenderTarget(const UINT & width = 0, const UINT & height = 0,
		const UINT & arraySize = 1, 
		const BOOL & createTexture = FALSE,
		const DXGI_FORMAT & format = DXGI_FORMAT_R8G8B8A8_UNORM);

	ID3D12Resource *const* GetResource() const;
	ID3D12DescriptorHeap * GetDescriptorHeap() const;

	const UINT & GetDescriptorSize() const;

	void SwitchToRTV(ID3D12GraphicsCommandList * commandList = nullptr);
	void SwitchToSRV(ID3D12GraphicsCommandList * commandList = nullptr);

	void SetGraphicsRootDescriptorTable(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList);

	void Clear(const CD3DX12_CPU_DESCRIPTOR_HANDLE & rtvHandle, ID3D12GraphicsCommandList * commandList = nullptr) const;
	void Release() override;

	

private:

	const float m_clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };

	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 1;
	D3D12_RESOURCE_STATES m_currentState[FRAME_BUFFER_COUNT] = { D3D12_RESOURCE_STATE_COMMON };

	UINT m_rtvDescriptorSize = 0;
	ID3D12Resource *		m_renderTargets[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12DescriptorHeap *	m_rtvDescriptorHeap = nullptr;
	SIZE_T m_descriptorHeapOffset = 0;

};

