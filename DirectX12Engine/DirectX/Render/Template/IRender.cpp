#include  "DirectX12EnginePCH.h"
#include "IRender.h"
#include "DirectX/Render/WrapperFunctions/Functions/Instancing.h"

void IRender::_updateWithThreads()
{
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
	{
		PRINT("FAILED TO SET PRIORITY LEVEL OF THREAD \n");
	}
	while (m_threadRunning)
	{	
		if (!m_threadDone)
		{
			this->Update(this->m_camera, this->m_deltaTime);
			this->Draw();
			m_threadDone = true;
		}
	}
	m_threadRunning = false;
}

IRender::IRender(RenderingManager* renderingManager,
                 const Window& window)
{
	this->p_renderingManager = renderingManager;
	this->p_window = &window;
	SAFE_NEW(p_drawQueue, new std::vector<Drawable*>());
	SAFE_NEW(p_lightQueue, new std::vector<ILight*>());
	SAFE_NEW(p_instanceGroups, new std::vector<Instancing::InstanceGroup>());

	m_threadRunning = true;
	m_threadDone = true;
	
	m_thread = std::thread(&IRender::_updateWithThreads, this);
}

IRender::~IRender()
{
	SAFE_DELETE(p_lightQueue);
	SAFE_DELETE(p_drawQueue);
	SAFE_DELETE(p_instanceGroups);
}

void IRender::ThreadUpdate(const Camera & camera, const float & deltaTime)
{
	if (p_commandList[*p_renderingManager->GetFrameIndex()] == nullptr)
		throw "Missing command list";

	if (m_threadDone && m_threadRunning && m_thread.get_id() != std::thread::id())
	{
		this->m_camera = camera;
		this->m_deltaTime = deltaTime;
		m_threadDone = false;
	}
	else 
	{
		if (m_thread.get_id() == std::thread::id())
		{
			m_threadRunning = true;
			m_thread = std::thread(&IRender::_updateWithThreads, this);
		}
		
		this->m_camera = camera;
		this->m_deltaTime = deltaTime;
		m_threadDone = false;
	}
}

#pragma optimize( "", off )
void IRender::ThreadJoin() const
{
	while (!m_threadDone);
}
#pragma optimize( "", on )

void IRender::KillThread()
{
	this->m_threadRunning = false;
	m_thread.join();
}

void IRender::Queue(Drawable* drawable) const
{
	p_drawQueue->push_back(drawable);
	Instancing::AddInstance(p_instanceGroups, drawable);
}

void IRender::QueueLight(ILight* light) const
{
	p_lightQueue->push_back(light);
}

HRESULT IRender::p_createCommandList(const std::wstring & name)
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

		SET_NAME(p_commandAllocator[i], name + L" Command allocator " + std::to_wstring(i));

		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommandList(
			0, 
			D3D12_COMMAND_LIST_TYPE_DIRECT, 
			p_commandAllocator[i], 
			nullptr, 
			IID_PPV_ARGS(&p_commandList[i]))))
		{		
			SET_NAME(p_commandList[i], name + L" Command list " + std::to_wstring(i));
			p_commandList[i]->Close();
		}
	}
	return hr;
}

void IRender::p_releaseCommandList()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(p_commandList[i]);
		SAFE_RELEASE(p_commandAllocator[i]);
	}
}

HRESULT IRender::p_createInstanceBuffer(const std::wstring & name, const UINT & bufferSize)
{
	HRESULT hr = 0;

	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		nullptr,
		IID_PPV_ARGS(&p_instanceBuffer))))
	{
		SET_NAME(p_instanceBuffer, name + L" intermediate INSTANCE BUFFER");
		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&p_intermediateInstanceBuffer))))
		{
			SET_NAME(p_intermediateInstanceBuffer, name + L" intermediate INSTANCE BUFFER");

		}
	}
	return hr;
}

UINT64 IRender::p_updateInstanceBuffer(D3D12_VERTEX_BUFFER_VIEW & vertexBufferView) const
{
	UINT64 bufferSize = 0;

	bufferSize = Instancing::UpdateInstanceGroup(p_commandList[*p_renderingManager->GetFrameIndex()],
		vertexBufferView,
		p_instanceBuffer,
		p_intermediateInstanceBuffer,
		p_instanceGroups,
		sizeof(Instancing::InstanceBuffer),
		bufferSize);
	
	if (bufferSize)
	{
		vertexBufferView.BufferLocation = p_instanceBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Instancing::InstanceBuffer);
		vertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
	}

	return bufferSize;
}

void IRender::p_drawInstance(const UINT & textureStartIndex, const BOOL& mapTextures) const
{
	ID3D12GraphicsCommandList * gcl = p_commandList[*p_renderingManager->GetFrameIndex()] ? p_commandList[*p_renderingManager->GetFrameIndex()] : p_renderingManager->GetCommandList();

	const size_t instanceGroupSize = p_instanceGroups->size();

	if (instanceGroupSize <= 0)
		return;

	D3D12_VERTEX_BUFFER_VIEW instanceBufferView = {};
	if (!p_updateInstanceBuffer(instanceBufferView))
		throw "FAILED TO UPDATE INSTANCE BUFFER";

	for (size_t i = 0; i < instanceGroupSize; i++)
	{		

		if (mapTextures)
			p_instanceGroups->at(i).MapTextures(textureStartIndex, p_commandList[*p_renderingManager->GetFrameIndex()]);

		D3D12_VERTEX_BUFFER_VIEW bufferArr[2] = 
			{ 
				p_instanceGroups->at(i).StaticMesh->GetVertexBufferView(),
				instanceBufferView
			};

		gcl->IASetVertexBuffers(0, 2, bufferArr);

		gcl->DrawInstanced(
			static_cast<UINT>(p_instanceGroups->at(i).StaticMesh->GetStaticMesh().size()),
			p_instanceGroups->at(i).GetSize(),
			0,
			static_cast<UINT>(i));

	}
}

void IRender::p_releaseInstanceBuffer()
{
	Instancing::ClearInstanceGroup(p_instanceGroups);
	SAFE_RELEASE(p_instanceBuffer);
	SAFE_RELEASE(p_intermediateInstanceBuffer);
}

HRESULT IRender::OpenCommandList(ID3D12PipelineState * pipelineState)
{
	HRESULT hr = 0;
	const UINT frameIndex = *p_renderingManager->GetFrameIndex();
	if (SUCCEEDED(hr = this->p_commandAllocator[frameIndex]->Reset()))
	{
		if (SUCCEEDED(hr = this->p_commandList[frameIndex]->Reset(this->p_commandAllocator[frameIndex], pipelineState)))
		{

		}
	}
	return hr;
}

HRESULT IRender::ExecuteCommandList() const
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = p_commandList[*p_renderingManager->GetFrameIndex()]->Close()))
	{
		ID3D12CommandList* ppCommandLists[] = { p_commandList[*p_renderingManager->GetFrameIndex()] };
		p_renderingManager->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	return hr;
}


