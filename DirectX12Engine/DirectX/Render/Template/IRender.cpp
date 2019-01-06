#include  "DirectX12EnginePCH.h"
#include "IRender.h"

IRender::IRender(RenderingManager* renderingManager,
                 const Window& window)
{
	this->p_renderingManager = renderingManager;
	this->p_window = &window;
	p_drawQueue = new std::vector<Drawable*>();
	p_lightQueue = new std::vector<ILight*>();
}

IRender::~IRender()
{
	SAFE_DELETE(p_lightQueue);
	SAFE_DELETE(p_drawQueue);
}

void IRender::Queue(Drawable* drawable) const
{
	p_drawQueue->push_back(drawable);
}

void IRender::QueueLight(ILight* light) const
{
	p_lightQueue->push_back(light);
}

HRESULT IRender::p_createCommandList()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = p_renderingManager->GetDevice()->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, 
			IID_PPV_ARGS(&p_commandAllocator[i]))))
		{
			return hr;
		}
	}
	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommandList(
		0, 
		D3D12_COMMAND_LIST_TYPE_DIRECT, 
		p_commandAllocator[0], 
		nullptr, 
		IID_PPV_ARGS(&p_commandList))))
	{		
		p_commandList->Close();
	}
	return hr;
}

void IRender::p_releaseCommandList()
{
	SAFE_RELEASE(p_commandList);
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(p_commandAllocator[i]);
	}
}

HRESULT IRender::OpenCommandList()
{
	HRESULT hr = 0;
	const UINT frameIndex = *p_renderingManager->GetFrameIndex();
	if (SUCCEEDED(hr = this->p_commandAllocator[frameIndex]->Reset()))
	{
		if (SUCCEEDED(hr = this->p_commandList->Reset(this->p_commandAllocator[frameIndex], nullptr)))
		{

		}
	}
	return hr;
}

HRESULT IRender::ExecuteCommandList() const
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = p_commandList->Close()))
	{
		ID3D12CommandList* ppCommandLists[] = { p_commandList };
		p_renderingManager->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	return hr;
}


