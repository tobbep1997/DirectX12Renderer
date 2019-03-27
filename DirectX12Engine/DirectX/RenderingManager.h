#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_5.h>
#include "Render/WrapperFunctions/X12Adapter.h"


class SSAOPass;
class DeferredRender;
class ShadowPass;
class GeometryPass;
class ParticlePass;
class ReflectionPass;
class Camera;

const unsigned int FRAME_BUFFER_COUNT = 3;
class RenderingManager
{
private:
	static RenderingManager * thisRenderingManager;
public:
	RenderingManager();
	~RenderingManager();

	static RenderingManager * GetInstance();
	static RenderingManager * GetPointerInstance();

	HRESULT Init(const Window * window, const BOOL & enableDebugLayer = FALSE);
	void Flush(const Camera * camera, const float & deltaTime, const BOOL & present = TRUE);
	void Present() const;
	void Release(const BOOL & waitForFrames = TRUE, const BOOL & reportMemoryLeaks = TRUE);
	void WaitForFrames();

	void UnsafeInit(const Window * window, const bool & enableDebugTools);

	X12Adapter * GetMainAdapter() const;
	X12Adapter * GetSecondAdapter() const;
	IDXGISwapChain4 * GetSwapChain() const;
	ID3D12GraphicsCommandList * GetCommandList() const;
	ID3D12CommandQueue * GetCommandQueue() const;
	ID3D12DescriptorHeap * GetRTVDescriptorHeap() const;

	ID3D12CommandAllocator * GetCommandAllocator()const;
	ID3D12Resource * GetRenderTargets() const;
	ID3D12Fence *	GetFence() const;

	UINT64 * GetFenceValues();
	const UINT & GetFrameIndex() const;
	const UINT & GetPrevFrameIndex() const;
	const UINT & GetRTVDescriptorSize() const;

	GeometryPass * GetGeometryPass() const;
	ShadowPass * GetShadowPass() const;
	DeferredRender * GetDeferredRender() const;
	ParticlePass * GetParticlePass() const;
	SSAOPass * GetSSAOPass() const;
	ReflectionPass * GetReflectionPass() const;


	HRESULT OpenCommandList();
	HRESULT SignalGPU();
	HRESULT SignalGPU(ID3D12GraphicsCommandList * commandList);
	   
	void ResourceDescriptorHeap(ID3D12GraphicsCommandList * commandList) const;

	D3D12_GPU_DESCRIPTOR_HANDLE CopyToGpuDescriptorHeap(const D3D12_CPU_DESCRIPTOR_HANDLE & descriptorHandle, const UINT & numDescriptors = 1);

private:
	   
	X12Adapter	*				m_mainAdapter = nullptr;
	X12Adapter	*				m_secondaryAdapter = nullptr;

	IDXGISwapChain4 *			m_swapChain = nullptr;
	
	ID3D12CommandQueue *		m_commandQueue = nullptr;
	ID3D12DescriptorHeap *		m_rtvDescriptorHeap = nullptr;

	HANDLE						m_fenceEvent;
	ID3D12GraphicsCommandList * m_commandList[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12CommandAllocator *	m_commandAllocator[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12Resource *			m_renderTargets[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12Fence *				m_fence[FRAME_BUFFER_COUNT]{ nullptr };
	UINT64						m_fenceValue[FRAME_BUFFER_COUNT]{ 0,0,0 };

	UINT m_frameIndex = 0;
	UINT m_prevFrameIndex = UINT_MAX;
	UINT m_rtvDescriptorSize = 0;

	HRESULT _flush(const Camera & camera, const float & deltaTime);
	HRESULT _present() const;
	void _clear();

	HRESULT _updatePipeline(const Camera & camera, const float & deltaTime);
	HRESULT _waitForPreviousFrame(const BOOL & updateFrame = TRUE, const BOOL & waitOnCpu = FALSE);

	HRESULT _checkAdapterSupport(IDXGIAdapter1 *& adapter, IDXGIFactory4 *& dxgiFactory, const UINT & adapterIndex = 0) const;
	HRESULT _createCommandQueue();
	HRESULT _createSwapChain(const Window & window, IDXGIFactory4 * dxgiFactory);
	HRESULT _createRenderTargetDescriptorHeap();
	HRESULT _createCommandAllocators();
	HRESULT _createCommandList();
	HRESULT _createFenceAndFenceEvent();

	GeometryPass *	m_geometryPass = nullptr;
	ShadowPass *	m_shadowPass = nullptr;
	DeferredRender * m_deferredPass = nullptr;
	ParticlePass * m_particlePass = nullptr;
	SSAOPass * m_ssaoPass = nullptr;
	ReflectionPass * m_reflectionPass = nullptr;

	SIZE_T m_copyOffset = 0;
	SIZE_T m_resourceIncrementalSize = 0;
	ID3D12DescriptorHeap * m_gpuDescriptorHeap = nullptr;

	HRESULT _createCbvSrvUavDescriptorHeap();

private:
	//DEBUG LAYER
	BOOL			m_debugLayerEnabled		= FALSE;
	ID3D12Debug *	m_debugLayer			= nullptr;
};

