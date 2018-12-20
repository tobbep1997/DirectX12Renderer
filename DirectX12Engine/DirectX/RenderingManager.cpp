#include "DirectX12EnginePCH.h"
#include "RenderingManager.h"
#include <functional>

#include "Render/GeometryPass.h"

RenderingManager::RenderingManager()
{
}

RenderingManager::~RenderingManager()
{
	this->Release();
}

RenderingManager* RenderingManager::GetInstance()
{
	static RenderingManager renderingManager;
	return &renderingManager;
}

HRESULT RenderingManager::Init(const Window & window, const BOOL & EnableDebugLayer)
{
	HRESULT hr;
	IDXGIAdapter1 * adapter = nullptr;
	IDXGIFactory4 * dxgiFactory = nullptr;

	m_geometryPass = new GeometryPass(this, window);

	if (SUCCEEDED(hr = this->_checkD3D12Support(adapter, dxgiFactory)))
	{
		if (EnableDebugLayer)
		{
			m_debugLayerEnabled = TRUE;
			if (SUCCEEDED(hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugLayer))))
			{
				m_debugLayer->EnableDebugLayer();
			}
			
			
		}
		if (SUCCEEDED(hr))
		{
			if (SUCCEEDED(hr = D3D12CreateDevice(
				adapter,
				D3D_FEATURE_LEVEL_12_0,
				IID_PPV_ARGS(&m_device))))
			{
				if (SUCCEEDED(hr = _createCommandQueue()))
				{
					if (SUCCEEDED(hr = _createSwapChain(window, dxgiFactory)))
					{
						if (SUCCEEDED(hr = _createRenderTargetDescriptorHeap()))
						{
							if (SUCCEEDED(hr = _createCommandAllocators()))
							{
								if (SUCCEEDED(hr = _createCommandList()))
								{
									if (SUCCEEDED(hr = _createFenceAndFenceEvent()))
									{						
										m_commandAllocator[this->m_frameIndex]->Reset();
										m_commandList->Reset(m_commandAllocator[this->m_frameIndex], nullptr);
										if (SUCCEEDED(hr = m_geometryPass->Init()))
										{

										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	SAFE_RELEASE(adapter);
	SAFE_RELEASE(dxgiFactory);

	if (FAILED(hr))
	{
		return Window::CreateError(hr);
	}	

	return hr;
}

void RenderingManager::Flush(const BOOL & present)
{
	HRESULT hr = 0;
	if (FAILED(hr = this->_flush()))
	{
		Window::CreateError(hr);
		Window::CloseWindow();
	}
	this->Present();
}

HRESULT RenderingManager::_updatePipeline()
{
	HRESULT hr = S_OK;
	if (FAILED(hr = _waitForPreviousFrame()))
	{
		return hr;
	}

	if (FAILED(hr = m_commandAllocator[m_frameIndex]->Reset()))
	{
		return hr;
	}
	if (FAILED(hr = m_commandList->Reset(m_commandAllocator[m_frameIndex], nullptr)))
	{
		return hr;
	}

	m_commandList->ResourceBarrier(1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderTargets[m_frameIndex],
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvDescriptorSize);

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	//---------------------------------------------------------------------
	//UPDATE HERE
	m_geometryPass->Draw();

	//---------------------------------------------------------------------

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderTargets[m_frameIndex],
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	m_commandList->Close();
	return hr;
}

HRESULT RenderingManager::_flush()
{
	HRESULT hr = 0;

	if (FAILED(hr = _updatePipeline()))
	{
		return hr;
	}

	ID3D12CommandList* ppCommandLists[] = { m_commandList };

	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	if (FAILED(hr = m_commandQueue->Signal(m_fence[m_frameIndex], m_fenceValue[m_frameIndex])))
	{
		return hr;
	}

	return hr;
}

HRESULT RenderingManager::_present()
{
	HRESULT hr = 0;
	hr = m_swapChain->Present(0, 0);
	return hr;
}

void RenderingManager::Present()
{
	HRESULT hr = 0;
	if (FAILED(hr = this->_present()))
	{
		Window::CreateError(hr);
		Window::CloseWindow();
	}
}

void RenderingManager::Release()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{		
		m_frameIndex = i;
		_waitForPreviousFrame(FALSE);
	}

	BOOL fs = false;
	if (m_swapChain->GetFullscreenState(&fs, NULL))
		m_swapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_commandQueue);
	SAFE_RELEASE(m_rtvDescriptorHeap);
	SAFE_RELEASE(m_commandList);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		SAFE_RELEASE(m_renderTargets[i]);
		SAFE_RELEASE(m_commandAllocator[i]);
		SAFE_RELEASE(m_fence[i]);
		m_fenceValue[i] = 0;
	};
	SAFE_RELEASE(m_debugLayer);

	m_frameIndex = 0;
	m_rtvDescriptorSize = 0;

	m_geometryPass->Release();
	delete m_geometryPass;
	m_geometryPass = nullptr;

	if (m_device->Release() > 0)
	{
		if (m_debugLayerEnabled)
		{
			ID3D12DebugDevice * dbgDevice = nullptr;
			if (SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&dbgDevice))))
			{
				dbgDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
				SAFE_RELEASE(dbgDevice);
			}
		}
	}
}

ID3D12Device* RenderingManager::GetDevice() const
{
	return this->m_device;
}

IDXGISwapChain3* RenderingManager::GetSwapChain() const
{
	return this->m_swapChain;
}

ID3D12GraphicsCommandList* RenderingManager::GetCommandList() const
{
	return this->m_commandList;
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

UINT* RenderingManager::GetFrameIndex()
{
	return &this->m_frameIndex;
}

UINT* RenderingManager::GetRTVDescriptorSize()
{
	return &this->m_rtvDescriptorSize;
}

UINT64 * RenderingManager::GetFenceValues()
{
	return this->m_fenceValue;
}

ID3D12CommandQueue* RenderingManager::GetCommandQueue() const
{
	return this->m_commandQueue;
}

HRESULT RenderingManager::_waitForPreviousFrame(const BOOL & updateFrame)
{
	HRESULT hr = 0;

	if (updateFrame)
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence[m_frameIndex]->GetCompletedValue() < m_fenceValue[m_frameIndex])
	{
		if (FAILED(hr = m_fence[m_frameIndex]->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent)))
		{
			return hr;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_fenceValue[m_frameIndex]++;

	return hr;
}

HRESULT RenderingManager::_checkD3D12Support(IDXGIAdapter1 *& adapter, IDXGIFactory4 *& dxgiFactory)
{
	HRESULT hr = 0;
	if (adapter || dxgiFactory)
		return E_INVALIDARG;

	if (SUCCEEDED(hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
	{
		hr = E_FAIL;

		UINT adapterIndex = 0;
		while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				adapterIndex++;
				continue;
			}

			if (SUCCEEDED(hr = D3D12CreateDevice(adapter, 
				D3D_FEATURE_LEVEL_12_0,
				_uuidof(ID3D12Device),
				nullptr)))
			{
				return S_OK;
				break;
			}
			SAFE_RELEASE(adapter);
		}

	}
	else
	{
		SAFE_RELEASE(dxgiFactory);
	}


	return hr;
}

HRESULT RenderingManager::_createCommandQueue()
{
	HRESULT hr = 0;

	D3D12_COMMAND_QUEUE_DESC desc = {};

	if (FAILED(hr = this->m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue))))
	{
		SAFE_RELEASE(this->m_commandQueue);
	}
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

	if (SUCCEEDED(hr = m_device->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&m_rtvDescriptorHeap))))
	{
		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			if (FAILED(hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]))))
			{
				break;
			}
			m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);
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
		if (FAILED(hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]))))
		{
			break;
		}
	}
	
	return hr;
}

HRESULT RenderingManager::_createCommandList()
{
	HRESULT hr = 0;

	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0], nullptr, IID_PPV_ARGS(&m_commandList));
	m_commandList->Close();
	return hr;
}

HRESULT RenderingManager::_createFenceAndFenceEvent()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i]))))
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
