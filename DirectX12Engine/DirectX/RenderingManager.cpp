#include "DirectX12EnginePCH.h"
#include "RenderingManager.h"
#include <functional>


RenderingManager::RenderingManager()
{
}


RenderingManager::~RenderingManager()
{
}

RenderingManager* RenderingManager::GetInstance()
{
	static RenderingManager renderingManager;
	return &renderingManager;
}


HRESULT RenderingManager::Init(const Window & window)
{
	HRESULT hr;
	IDXGIAdapter1 * adapter = nullptr;
	IDXGIFactory4 * dxgiFactory = nullptr;
	if (SUCCEEDED(hr = this->_checkD3D12Support(adapter, dxgiFactory)))
	{
		if (SUCCEEDED(hr = D3D12CreateDevice(
			adapter,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device))))
		{
			if (SUCCEEDED(hr = _createCommandQueue()))
			{
				if (SUCCEEDED(hr = _createSwapChain(window, dxgiFactory)))
				{
					
				}
			}
		}
	}

	SAFE_RELEASE(adapter);
	SAFE_RELEASE(dxgiFactory);
	return hr;
}

HRESULT RenderingManager::Update()
{
	return S_OK;

}

HRESULT RenderingManager::Flush()
{
	return S_OK;

}

HRESULT RenderingManager::Present()
{
	return S_OK;

}

HRESULT RenderingManager::Release()
{
	return S_OK;

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
				D3D_FEATURE_LEVEL_11_0,
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

	if (FAILED(hr))
	{	
		return Window::CreateError(hr);
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
		return Window::CreateError(hr);
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

	IDXGISwapChain * tmpSwapChain;

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
	if (FAILED(hr))
	{
		return Window::CreateError(hr);
	}
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
	}

	if (FAILED(hr))
	{
		return Window::CreateError(hr);
	}
	return hr;
}
