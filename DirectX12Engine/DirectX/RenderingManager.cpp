#include "DirectX12EnginePCH.h"
#include "RenderingManager.h"

#include "Render/GeometryPass.h"
#include "Render/ShadowPass.h"
#include "Render/DeferredRender.h"
#include "Render/ParticlePass.h"

#include "Render/SSAOPass.h"
#include "Render/ReflectionPass.h"

RenderingManager * RenderingManager::thisRenderingManager = nullptr;

RenderingManager::RenderingManager()
= default;

RenderingManager::~RenderingManager()
= default;

RenderingManager* RenderingManager::GetInstance()
{
	static RenderingManager renderingManager;
	return &renderingManager;
}

RenderingManager* RenderingManager::GetPointerInstance()
{
	if (!thisRenderingManager)
		thisRenderingManager = new RenderingManager();
	return thisRenderingManager;
}

HRESULT RenderingManager::Init(const Window * window, const BOOL & enableDebugLayer)
{
	HRESULT hr;
	IDXGIAdapter1 * adapter = nullptr, * adapter1 = nullptr;
	IDXGIFactory4 * dxgiFactory = nullptr;



	if (enableDebugLayer)
	{
		m_debugLayerEnabled = TRUE;
		if (SUCCEEDED(hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugLayer))))
		{
			m_debugLayer->EnableDebugLayer();
		}
	}
	if (SUCCEEDED(hr = this->_checkAdapterSupport(adapter, dxgiFactory, 0)))
	{	
		SAFE_NEW(m_mainAdapter, new X12Adapter());
		if (SUCCEEDED(hr = m_mainAdapter->CreateDevice(adapter)))
		{
			if (SUCCEEDED(hr = this->_checkAdapterSupport(adapter1, dxgiFactory, 1)))
			{
				SAFE_NEW(m_secondaryAdapter, new X12Adapter());
				if (FAILED(hr = m_secondaryAdapter->CreateDevice(adapter1)))
				{
					return hr;
				}

			}				

			if (FAILED(hr = _createCbvSrvUavDescriptorHeap()))
			{
				return Window::CreateError(hr);
			}
			if (FAILED(hr = _createCommandQueue()))
			{
				return Window::CreateError(hr);
			}
			if (FAILED(hr = _createSwapChain(*window, dxgiFactory)))
			{
				return Window::CreateError(hr);
			}
			if (FAILED(hr = _createRenderTargetDescriptorHeap()))
			{
				return Window::CreateError(hr);
			}
			if (FAILED(hr = _createCommandAllocators()))
			{
				return Window::CreateError(hr);
			}
			if (FAILED(hr = _createCommandList()))
			{
				return Window::CreateError(hr);
			}
			if (FAILED(hr = _createFenceAndFenceEvent()))
			{
				return Window::CreateError(hr);
			}
			
			SAFE_NEW(m_geometryPass, new GeometryPass(this, *window));
			if (FAILED(hr = m_geometryPass->Init()))
			{
				return Window::CreateError(hr);
			}
			SAFE_NEW(m_shadowPass, new ShadowPass(this, *window));
			if (FAILED(hr = m_shadowPass->Init()))
			{
				return Window::CreateError(hr);
			}
			SAFE_NEW(m_deferredPass, new DeferredRender(this, *window));
			if (FAILED(hr = m_deferredPass->Init()))
			{
				return Window::CreateError(hr);
			}
			SAFE_NEW(m_particlePass, new ParticlePass(this, *window));
			if (FAILED(hr = m_particlePass->Init()))
			{
				return Window::CreateError(hr);
			}
			SAFE_NEW(m_ssaoPass, new SSAOPass(this, *window));
			if (FAILED(hr = m_ssaoPass->Init()))
			{
				return Window::CreateError(hr);
			}
			SAFE_NEW(m_reflectionPass, new ReflectionPass(this, *window));
			if (FAILED(hr = m_reflectionPass->Init()))
			{
				return Window::CreateError(hr);
			}
		}		
	}


	SAFE_RELEASE(adapter);
	SAFE_RELEASE(adapter1);
	SAFE_RELEASE(dxgiFactory);
	
	if (FAILED(hr))
	{
		return Window::CreateError(hr);
	}


	return hr;
}

void RenderingManager::Flush(const Camera * camera, const float & deltaTime, const BOOL & present)
{
	HRESULT hr = 0;
	bool createdCamera = true;
	if (camera)
		createdCamera = false;
	const Camera * cam = camera ? camera : new Camera(DirectX::XMFLOAT4(0,0,-5,1));
	
	if (FAILED(hr = this->_flush(*cam, deltaTime)))
	{
		Window::CreateError(hr);
		Window::CloseWindow();
	}
	if (createdCamera)
		delete cam;
	this->Present();
}

HRESULT RenderingManager::_updatePipeline(const Camera & camera, const float & deltaTime)
{
	HRESULT hr = S_OK;

	if (FAILED(hr = _waitForPreviousFrame(TRUE, TRUE)))
	{
		return hr;
	}
	if (FAILED(hr = m_commandAllocator[m_frameIndex]->Reset()))
	{
		return hr;
	}
	if (FAILED(hr = m_commandList[m_frameIndex]->Reset(m_commandAllocator[m_frameIndex], nullptr)))
	{
		return hr;
	}
		
	m_commandList[m_frameIndex]->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderTargets[m_frameIndex],
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//---------------------------------------------------------------------

	ResourceDescriptorHeap(m_commandList[m_frameIndex]);

	m_particlePass->ThreadUpdate(camera, deltaTime);
	m_shadowPass->ThreadUpdate(camera, deltaTime);
	
	m_particlePass->ThreadJoin();
	
	m_geometryPass->ThreadUpdate(camera, deltaTime);
	m_geometryPass->ThreadJoin();
	
	//m_reflectionPass->ThreadUpdate(camera, deltaTime);
	m_ssaoPass->ThreadUpdate(camera, deltaTime);
	
	m_shadowPass->ThreadJoin();
	m_ssaoPass->ThreadJoin();
	//m_reflectionPass->ThreadJoin();
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvDescriptorSize);

	m_commandList[m_frameIndex]->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	m_commandList[m_frameIndex]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_deferredPass->Update(camera, deltaTime);
	m_deferredPass->Draw();
	//---------------------------------------------------------------------

	m_commandList[m_frameIndex]->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderTargets[m_frameIndex],
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	m_commandList[m_frameIndex]->Close();

	return hr;
}

HRESULT RenderingManager::_flush(const Camera & camera, const float & deltaTime)
{
	HRESULT hr = 0;

	if (FAILED(hr = _updatePipeline(camera, deltaTime)))
	{
		return hr;
	}

	ID3D12CommandList* ppCommandLists[] = { m_commandList[m_frameIndex] };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (FAILED(hr = m_commandQueue->Signal(m_fence[m_frameIndex], m_fenceValue[m_frameIndex])))
	{
		return hr;
	}


	_clear();
	return hr;
}

HRESULT RenderingManager::_present() const
{
	HRESULT hr = 0;
	DXGI_PRESENT_PARAMETERS pp{};
	hr = m_swapChain->Present1(0, 0, &pp);
	return hr;
}

void RenderingManager::_clear()
{
	m_geometryPass->Clear();
	m_shadowPass->Clear();
	m_deferredPass->Clear();
	m_particlePass->Clear();
	m_ssaoPass->Clear();
	m_reflectionPass->Clear();

	m_copyOffset = 0;
}

void RenderingManager::Present() const
{
	HRESULT hr = 0;
	if (FAILED(hr = this->_present()))
	{
		Window::CreateError(hr);
		Window::CloseWindow();
	}
}

void RenderingManager::Release(const BOOL & waitForFrames, const BOOL & reportMemoryLeaks)
{
	if (waitForFrames)
		WaitForFrames();

	BOOL fullscreen = FALSE;
	m_swapChain->GetFullscreenState(&fullscreen, nullptr);
	if (fullscreen)
		m_swapChain->SetFullscreenState(FALSE, nullptr);
	   
	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_commandQueue);
	SAFE_RELEASE(m_rtvDescriptorHeap);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		SAFE_RELEASE(m_commandList[i]);
		SAFE_RELEASE(m_renderTargets[i]);
		SAFE_RELEASE(m_commandAllocator[i]);
		SAFE_RELEASE(m_fence[i]);
		m_fenceValue[i] = 0;
	};
	SAFE_RELEASE(m_debugLayer);
	SAFE_RELEASE(m_gpuDescriptorHeap);



	m_frameIndex = 0;
	m_rtvDescriptorSize = 0;

	m_geometryPass->KillThread();
	m_geometryPass->Release();
	SAFE_DELETE(m_geometryPass);
	
	m_deferredPass->KillThread();
	m_deferredPass->Release();
	SAFE_DELETE(m_deferredPass);
	
	m_reflectionPass->KillThread();
	m_reflectionPass->Release();
	SAFE_DELETE(m_reflectionPass);
	
	m_shadowPass->KillThread();
	m_shadowPass->Release();
	SAFE_DELETE(m_shadowPass);
	
	m_particlePass->KillThread();
	m_particlePass->Release();
	SAFE_DELETE(m_particlePass);
	
	m_ssaoPass->KillThread();
	m_ssaoPass->Release();
	SAFE_DELETE(m_ssaoPass);

	
	m_secondaryAdapter->Release();
	SAFE_DELETE(m_secondaryAdapter);
	if (m_mainAdapter->Release())
	{
		if (m_debugLayerEnabled && reportMemoryLeaks)
		{
			ID3D12DebugDevice * dbgDevice = nullptr;
			if (SUCCEEDED(m_mainAdapter->GetDevice()->QueryInterface(IID_PPV_ARGS(&dbgDevice))))
			{
				dbgDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
				SAFE_RELEASE(dbgDevice);
			}
		}
	}
	SAFE_DELETE(m_mainAdapter);

}

void RenderingManager::WaitForFrames()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		m_frameIndex = i;
		_waitForPreviousFrame(FALSE, TRUE);
	}
}

void RenderingManager::UnsafeInit(const Window* window, const bool& enableDebugTools)
{
	this->Init(window, enableDebugTools);
}

X12Adapter* RenderingManager::GetSecondAdapter() const
{
	return m_secondaryAdapter;
}

X12Adapter* RenderingManager::GetMainAdapter() const
{
	return m_mainAdapter;
}

IDXGISwapChain4* RenderingManager::GetSwapChain() const
{
	return this->m_swapChain;
}

ID3D12GraphicsCommandList* RenderingManager::GetCommandList() const
{
	return this->m_commandList[m_frameIndex];
}

ID3D12DescriptorHeap* RenderingManager::GetRTVDescriptorHeap() const
{
	return this->m_rtvDescriptorHeap;
}

ID3D12CommandAllocator* RenderingManager::GetCommandAllocator() const
{
	return *this->m_commandAllocator;
}

ID3D12Resource* RenderingManager::GetRenderTargets() const
{
	return *this->m_renderTargets;
}

ID3D12Fence* RenderingManager::GetFence() const
{
	return *this->m_fence;
}

const UINT & RenderingManager::GetFrameIndex() const
{
	return this->m_frameIndex;
}

const UINT& RenderingManager::GetPrevFrameIndex() const
{
	return this->m_prevFrameIndex;
}

const UINT & RenderingManager::GetRTVDescriptorSize() const
{
	return this->m_rtvDescriptorSize;
}

GeometryPass* RenderingManager::GetGeometryPass() const
{
	return this->m_geometryPass;
}

ShadowPass* RenderingManager::GetShadowPass() const
{
	return this->m_shadowPass;
}

DeferredRender* RenderingManager::GetDeferredRender() const
{
	return this->m_deferredPass;
}

ParticlePass* RenderingManager::GetParticlePass() const
{
	return this->m_particlePass;
}

SSAOPass* RenderingManager::GetSSAOPass() const
{
	return this->m_ssaoPass;
}

ReflectionPass* RenderingManager::GetReflectionPass() const
{
	return this->m_reflectionPass;
}

HRESULT RenderingManager::OpenCommandList()
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = this->m_commandAllocator[m_frameIndex]->Reset()))
	{
		if (SUCCEEDED(hr = this->m_commandList[m_frameIndex]->Reset(this->m_commandAllocator[m_frameIndex], nullptr)))
		{

		}
	}
	return hr;
}

HRESULT RenderingManager::SignalGPU()
{
	HRESULT hr = 0;
	this->m_commandList[m_frameIndex]->Close();
	ID3D12CommandList* ppCommandLists[] = { this->m_commandList[m_frameIndex] };
	this->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	
	this->m_fenceValue[this->m_frameIndex]++;
	if (SUCCEEDED(hr = this->m_commandQueue->Signal(
		&GetFence()[this->m_frameIndex],
		this->m_fenceValue[this->m_frameIndex])))
	{

	}
	return hr;
}

HRESULT RenderingManager::SignalGPU(ID3D12GraphicsCommandList* commandList)
{
	HRESULT hr = 0;
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	this->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	this->m_fenceValue[this->m_frameIndex]++;
	if (SUCCEEDED(hr = this->m_commandQueue->Signal(
		&GetFence()[this->m_frameIndex],
		this->m_fenceValue[this->m_frameIndex])))
	{

	}
	return hr;
}


void RenderingManager::ResourceDescriptorHeap(ID3D12GraphicsCommandList* commandList) const
{
	ID3D12DescriptorHeap* DescriptorHeaps[] = { m_gpuDescriptorHeap };
	commandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);
}

D3D12_GPU_DESCRIPTOR_HANDLE RenderingManager::CopyToGpuDescriptorHeap(
	const D3D12_CPU_DESCRIPTOR_HANDLE& descriptorHandle, const UINT& numDescriptors)
{
	const SIZE_T offset = m_copyOffset;
	const D3D12_CPU_DESCRIPTOR_HANDLE destHandle = { m_gpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_copyOffset };

	m_mainAdapter->GetDevice()->CopyDescriptorsSimple(
		numDescriptors,
		destHandle,
		descriptorHandle,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	m_copyOffset += m_resourceIncrementalSize * numDescriptors;

	return { m_gpuDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + offset };
}

UINT64 * RenderingManager::GetFenceValues()
{
	return this->m_fenceValue;
}

ID3D12CommandQueue* RenderingManager::GetCommandQueue() const
{
	return this->m_commandQueue;
}

HRESULT RenderingManager::_waitForPreviousFrame(const BOOL & updateFrame, const BOOL & waitOnCpu)
{
	HRESULT hr = 0;
	m_prevFrameIndex = m_frameIndex;
	if (updateFrame)
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();


	if (m_fence[m_frameIndex]->GetCompletedValue() < m_fenceValue[m_frameIndex])
	{
		if (FAILED(hr = m_fence[m_frameIndex]->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent)))
		{
			return hr;
		}
		if (!waitOnCpu)
			m_commandQueue->Wait(m_fence[m_frameIndex], m_fenceValue[m_frameIndex]);
		else
			WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_fenceValue[m_frameIndex]++;
	


	return hr;
}

HRESULT RenderingManager::_checkAdapterSupport(IDXGIAdapter1*& adapter, IDXGIFactory4*& dxgiFactory,
	const UINT& adapterIndex) const
{
	HRESULT hr = 0;
	if (adapter)
		return E_INVALIDARG;

	if (!dxgiFactory)
		if (FAILED(hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
			return hr;


	hr = E_FAIL;
	if (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{				
			return E_FAIL;
		}

		if (SUCCEEDED(hr = D3D12CreateDevice(adapter,
			D3D_FEATURE_LEVEL_12_0,
			_uuidof(ID3D12Device),
			nullptr)))
		{				
			return S_OK;				
		}
		
		SAFE_RELEASE(adapter);		
	}
	return hr;
}

HRESULT RenderingManager::_createCommandQueue()
{
	HRESULT hr = 0;

	D3D12_COMMAND_QUEUE_DESC desc = {};

	if (FAILED(hr = this->m_mainAdapter->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue))))
	{
		SAFE_RELEASE(this->m_commandQueue);
	}
	SET_NAME(m_commandQueue, L"Default commandQueue");
	return hr;
}

HRESULT RenderingManager::_createSwapChain(const Window & window, IDXGIFactory4 * dxgiFactory)
{
	HRESULT hr = 0;
	if (!dxgiFactory)
		return E_INVALIDARG;

	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = window.GetWidth();
	backBufferDesc.Height = window.GetHeight();
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FRAME_BUFFER_COUNT;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = window.GetHWND();
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.Windowed = !window.GetFullscreen();

	IDXGISwapChain * tmpSwapChain = nullptr;
	

	if (SUCCEEDED(hr = dxgiFactory->CreateSwapChain(m_commandQueue, 
		&swapChainDesc,
		&tmpSwapChain)))
	{
		if (FAILED(hr = tmpSwapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain))))
		{
			SAFE_RELEASE(m_swapChain);
			return E_FAIL;
		}
	}
	SAFE_RELEASE(tmpSwapChain);
	return hr;
}

HRESULT RenderingManager::_createRenderTargetDescriptorHeap()
{
	HRESULT hr = 0;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (SUCCEEDED(hr = m_mainAdapter->GetDevice()->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&m_rtvDescriptorHeap))))
	{
		m_rtvDescriptorSize = m_mainAdapter->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			if (FAILED(hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]))))
			{
				break;
			}
			m_mainAdapter->GetDevice()->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}

	}
	return hr;
}

HRESULT RenderingManager::_createCommandAllocators()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_mainAdapter->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]))))
		{
			break;
		}
		SET_NAME(m_commandAllocator[i], L"Default commandAllocator " + std::to_wstring(i));

	}
	
	return hr;
}

HRESULT RenderingManager::_createCommandList()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		hr = m_mainAdapter->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0], nullptr, IID_PPV_ARGS(&m_commandList[i]));
		m_commandList[i]->Close();
		SET_NAME(m_commandList[i], L"Default commandList " + std::to_wstring(i));

	}
	return hr;
}

HRESULT RenderingManager::_createFenceAndFenceEvent()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_mainAdapter->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i]))))
		{
			break;
		}
		m_fenceValue[i] = 0;
	}

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (nullptr == m_fenceEvent)
		return E_FAIL;

	return hr;
}

HRESULT RenderingManager::_createCbvSrvUavDescriptorHeap()
{
	HRESULT hr = 0;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeap{};

	descriptorHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeap.NumDescriptors = MAX_DESCRIPTOR_SIZE;
	   
	if (FAILED(hr = m_mainAdapter->GetDevice()->CreateDescriptorHeap(
		&descriptorHeap, 
		IID_PPV_ARGS(&m_gpuDescriptorHeap))))
	{
		return hr;
	}

	m_resourceIncrementalSize = m_mainAdapter->GetDescriptorHandleIncrementSize();

	return hr;
}
