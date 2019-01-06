#pragma once
#include "DirectX12EnginePCH.h"

class DeferredRender;
class ShadowPass;
class GeometryPass;
class Camera;

const UINT FRAME_BUFFER_COUNT = 3;
class RenderingManager
{
public:
	RenderingManager();
	~RenderingManager();

	static RenderingManager * GetInstance();

	HRESULT Init(const Window & window, const BOOL & EnableDebugLayer = FALSE);
	void Flush(const Camera & camera, const BOOL & present = TRUE);
	void Present() const;
	void Release(const BOOL & waitForFrames = TRUE, const BOOL & reportMemoryLeaks = TRUE);
	void WaitForFrames();


	ID3D12Device * GetDevice() const;
	IDXGISwapChain3 * GetSwapChain() const;
	ID3D12GraphicsCommandList * GetCommandList() const;
	ID3D12CommandQueue * GetCommandQueue() const;
	ID3D12DescriptorHeap * GetRTVDescriptorHeap() const;

	ID3D12CommandAllocator * GetCommandAllocator()const;
	ID3D12Resource * GetRenderTargets() const;
	ID3D12Fence *	GetFence() const;

	UINT64 * GetFenceValues();
	UINT * GetFrameIndex();
	UINT * GetRTVDescriptorSize();

	GeometryPass * GetGeometryPass() const;
	ShadowPass * GetShadowPass() const;
	DeferredRender * GetDeferredRender() const;

	HRESULT OpenCommandList();
	HRESULT SignalGPU();
	HRESULT SignalGPU(ID3D12GraphicsCommandList * commandList);

private:

	ID3D12Device *				m_device = nullptr;
	IDXGISwapChain3 *			m_swapChain = nullptr;
	ID3D12GraphicsCommandList * m_commandList = nullptr;
	ID3D12CommandQueue *		m_commandQueue = nullptr;
	ID3D12DescriptorHeap *		m_rtvDescriptorHeap = nullptr;

	HANDLE						m_fenceEvent;
	ID3D12CommandAllocator *	m_commandAllocator[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12Resource *			m_renderTargets[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12Fence *				m_fence[FRAME_BUFFER_COUNT]{ nullptr };
	UINT64						m_fenceValue[FRAME_BUFFER_COUNT]{ 0,0,0 };


	UINT m_frameIndex = 0;
	UINT m_rtvDescriptorSize = 0;

	HRESULT _flush(const Camera & camera);
	HRESULT _present() const;
	void _clear() const;

	HRESULT _updatePipeline(const Camera & camera);
	HRESULT _waitForPreviousFrame(const BOOL & updateFrame = TRUE);

	HRESULT _checkD3D12Support(IDXGIAdapter1 *& adapter, IDXGIFactory4 *& dxgiFactory) const;
	HRESULT _createCommandQueue();
	HRESULT _createSwapChain(const Window & window, IDXGIFactory4 * dxgiFactory);
	HRESULT _createRenderTargetDescriptorHeap();
	HRESULT _createCommandAllocators();
	HRESULT _createCommandList();
	HRESULT _createFenceAndFenceEvent();

	GeometryPass *	m_geometryPass = nullptr;
	ShadowPass *	m_shadowPass = nullptr;
	DeferredRender * m_deferredPass = nullptr;

private:
	//DEBUG LAYER
	BOOL			m_debugLayerEnabled		= FALSE;
	ID3D12Debug *	m_debugLayer			= nullptr;
};

