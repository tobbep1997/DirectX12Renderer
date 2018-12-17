#pragma once
#include "DirectX12EnginePCH.h"

const UINT FRAME_BUFFER_COUNT = 3;
class RenderingManager
{
public:
	RenderingManager();
	~RenderingManager();

	static RenderingManager * GetInstance();

	HRESULT Init(const Window & window, const BOOL & EnableDebugLayer = FALSE);
	void Flush(const BOOL & present = TRUE);
	void Present();
	void Release();

private:

	ID3D12Device *				m_device = nullptr;
	IDXGISwapChain3 *			m_swapChain = nullptr;
	ID3D12GraphicsCommandList * m_commandList = nullptr;
	ID3D12CommandQueue *		m_commandQueue = nullptr;
	ID3D12DescriptorHeap *		m_rtvDescriptorHeap = nullptr;

	HANDLE						m_fenceEvent;
	ID3D12CommandAllocator *	m_commandAllocator[FRAME_BUFFER_COUNT]{nullptr, nullptr, nullptr};
	ID3D12Resource *			m_renderTargets[FRAME_BUFFER_COUNT]{ nullptr, nullptr, nullptr };
	ID3D12Fence *				m_fence[FRAME_BUFFER_COUNT]{ nullptr, nullptr, nullptr };
	UINT64						m_fenceValue[FRAME_BUFFER_COUNT]{ 0,0,0 };


	UINT m_frameIndex = 0;
	UINT m_rtvDescriptorSize = 0;

	HRESULT _flush();
	HRESULT _present();


	HRESULT _updatePipeline();
	HRESULT _waitForPreviousFrame(const BOOL & updateFrame = TRUE);

	HRESULT _checkD3D12Support(IDXGIAdapter1 *& adapter, IDXGIFactory4 *& dxgiFactory);
	HRESULT _createCommandQueue();
	HRESULT _createSwapChain(const Window & window, IDXGIFactory4 * dxgiFactory);
	HRESULT _createRenderTargetDescriptorHeap();
	HRESULT _createCommandAllocators();
	HRESULT _createCommandList();
	HRESULT _createFenceAndFenceEvent();

private:
	//DEBUG LAYER
	BOOL			m_debugLayerEnabled		= FALSE;
	ID3D12Debug *	m_debugLayer			= nullptr;
};

