#pragma once
#include "DirectX12EnginePCH.h"

const UINT FRAME_BUFFER_COUNT = 3;
class RenderingManager
{
public:
	RenderingManager();
	~RenderingManager();

	static RenderingManager * GetInstance();

	HRESULT Init(const Window & window);
	HRESULT Update();
	HRESULT Flush();
	HRESULT Present();
	HRESULT Release();

private:

	ID3D12Device *				m_device = nullptr;
	IDXGISwapChain3 *			m_swapChain = nullptr;
	ID3D12CommandQueue *		m_commandQueue = nullptr;
	ID3D12DescriptorHeap *		m_rtvDescriptorHeap = nullptr;
	ID3D12CommandAllocator *	m_commandAllocator = nullptr;
	ID3D12GraphicsCommandList * m_commandList = nullptr;
	ID3D12Resource *			m_renderTargets[FRAME_BUFFER_COUNT]{ nullptr, nullptr, nullptr };

	ID3D12Fence *				m_fence = nullptr;
	HANDLE						m_fenceHandle;
	UINT64						m_fenceValue[FRAME_BUFFER_COUNT];

	int m_frameIndex = 0;
	int m_rtvDescriptorSize = 0;


	HRESULT _checkD3D12Support(IDXGIAdapter1 *& adapter, IDXGIFactory4 *& dxgiFactory);
	HRESULT _createCommandQueue();
	HRESULT _createSwapChain(const Window & window, IDXGIFactory4 * dxgiFactory);
	HRESULT _createRenderTargetDescriptorHeap();
};

